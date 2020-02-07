//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2018-2020 Nil Foundation AG
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/mtl/network/multiplexer.hpp>

#include <algorithm>

#include <nil/mtl/byte.hpp>
#include <nil/mtl/config.hpp>
#include <nil/mtl/error.hpp>
#include <nil/mtl/expected.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/network/operation.hpp>
#include <nil/mtl/network/pollset_updater.hpp>
#include <nil/mtl/network/socket_manager.hpp>
#include <nil/mtl/sec.hpp>
#include <nil/mtl/variant.hpp>

#ifndef MTL_WINDOWS
#include <poll.h>
#else
#include "caf/detail/socket_sys_includes.hpp"
#endif    // MTL_WINDOWS

namespace nil {
    namespace mtl {
        namespace network {

#ifndef POLLRDHUP
#define POLLRDHUP POLLHUP
#endif

#ifndef POLLPRI
#define POLLPRI POLLIN
#endif

            namespace {

#ifdef MTL_WINDOWS
                // From the MSDN: If the POLLPRI flag is set on a socket for the Microsoft
                //                Winsock provider, the WSAPoll function will fail.
                const short input_mask = POLLIN;
#else
                const short input_mask = POLLIN | POLLPRI;
#endif

                const short error_mask = POLLRDHUP | POLLERR | POLLHUP | POLLNVAL;

                const short output_mask = POLLOUT;

                short to_bitmask(operation op) {
                    switch (op) {
                        case operation::read:
                            return input_mask;
                        case operation::write:
                            return output_mask;
                        case operation::read_write:
                            return input_mask | output_mask;
                        default:
                            return 0;
                    }
                }

            }    // namespace

            multiplexer::multiplexer() : shutting_down_(false) {
                // nop
            }

            multiplexer::~multiplexer() {
                // nop
            }

            error multiplexer::init() {
                auto pipe_handles = make_pipe();
                if (!pipe_handles)
                    return std::move(pipe_handles.error());
                add(make_counted<pollset_updater>(pipe_handles->first, shared_from_this()));
                write_handle_ = pipe_handles->second;
                return none;
            }

            size_t multiplexer::num_socket_managers() const noexcept {
                return managers_.size();
            }

            ptrdiff_t multiplexer::index_of(const socket_manager_ptr &mgr) {
                auto first = managers_.begin();
                auto last = managers_.end();
                auto i = std::find(first, last, mgr);
                return i == last ? -1 : std::distance(first, i);
            }

            void multiplexer::register_reading(const socket_manager_ptr &mgr) {
                if (std::this_thread::get_id() == tid_) {
                    if (shutting_down_) {
                        // discard
                    } else if (mgr->mask() != operation::none) {
                        MTL_ASSERT(index_of(mgr) != -1);
                        if (mgr->mask_add(operation::read)) {
                            auto &fd = pollset_[index_of(mgr)];
                            fd.events |= input_mask;
                        }
                    } else if (mgr->mask_add(operation::read)) {
                        add(mgr);
                    }
                } else {
                    write_to_pipe(0, mgr);
                }
            }

            void multiplexer::register_writing(const socket_manager_ptr &mgr) {
                if (std::this_thread::get_id() == tid_) {
                    if (shutting_down_) {
                        // discard
                    } else if (mgr->mask() != operation::none) {
                        MTL_ASSERT(index_of(mgr) != -1);
                        if (mgr->mask_add(operation::write)) {
                            auto &fd = pollset_[index_of(mgr)];
                            fd.events |= output_mask;
                        }
                    } else if (mgr->mask_add(operation::write)) {
                        add(mgr);
                    }
                } else {
                    write_to_pipe(1, mgr);
                }
            }

            void multiplexer::close_pipe() {
                std::lock_guard<std::mutex> guard {write_lock_};
                if (write_handle_ != invalid_socket) {
                    close(write_handle_);
                    write_handle_ = pipe_socket {};
                }
            }

            bool multiplexer::poll_once(bool blocking) {
                if (pollset_.empty())
                    return false;
                // We'll call poll() until poll() succeeds or fails.
                for (;;) {
                    int presult;
#ifdef MTL_WINDOWS
                    presult = ::WSAPoll(pollset_.data(), static_cast<ULONG>(pollset_.size()), blocking ? -1 : 0);
#else
                    presult = ::poll(pollset_.data(), static_cast<nfds_t>(pollset_.size()), blocking ? -1 : 0);
#endif
                    if (presult < 0) {
                        auto code = last_socket_error();
                        switch (code) {
                            case std::errc::interrupted: {
                                // A signal was caught. Simply try again.
                                MTL_LOG_DEBUG("received errc::interrupted, try again");
                                break;
                            }
                            case std::errc::not_enough_memory: {
                                MTL_LOG_ERROR("poll() failed due to insufficient memory");
                                // There's not much we can do other than try again in hope someone
                                // else releases memory.
                                break;
                            }
                            default: {
                                // Must not happen.
                                auto int_code = static_cast<int>(code);
                                auto msg = std::generic_category().message(int_code);
                                string_view prefix = "poll() failed: ";
                                msg.insert(msg.begin(), prefix.begin(), prefix.end());
                                MTL_CRITICAL(msg.c_str());
                            }
                        }
                        // Rinse and repeat.
                        continue;
                    }
                    MTL_LOG_DEBUG("poll() on" << pollset_.size() << "sockets reported" << presult << "event(s)");
                    // No activity.
                    if (presult == 0)
                        return false;
                    // Scan pollset for events.
                    MTL_LOG_DEBUG("scan pollset for socket events");
                    for (size_t i = 0; i < pollset_.size() && presult > 0;) {
                        auto revents = pollset_[i].revents;
                        if (revents != 0) {
                            auto events = pollset_[i].events;
                            auto mgr = managers_[i];
                            auto new_events = handle(mgr, events, revents);
                            --presult;
                            if (new_events == 0) {
                                del(i);
                                continue;
                            } else if (new_events != events) {
                                pollset_[i].events = new_events;
                            }
                        }
                        ++i;
                    }
                    return true;
                }
            }

            void multiplexer::set_thread_id() {
                tid_ = std::this_thread::get_id();
            }

            void multiplexer::run() {
                MTL_LOG_TRACE("");
                while (!pollset_.empty())
                    poll_once(true);
            }

            void multiplexer::shutdown() {
                if (std::this_thread::get_id() == tid_) {
                    shutting_down_ = true;
                    // First manager is the pollset_updater. Skip it and delete later.
                    for (size_t i = 1; i < managers_.size();) {
                        auto &mgr = managers_[i];
                        if (mgr->mask_del(operation::read)) {
                            auto &fd = pollset_[index_of(mgr)];
                            fd.events &= ~input_mask;
                        }
                        if (mgr->mask() == operation::none)
                            del(i);
                        else
                            ++i;
                    }
                    close_pipe();
                } else {
                    write_to_pipe(4, nullptr);
                }
            }

            short multiplexer::handle(const socket_manager_ptr &mgr, short events, short revents) {
                MTL_LOG_TRACE(MTL_ARG2("socket", mgr->handle()));
                MTL_ASSERT(mgr != nullptr);
                bool checkerror = true;
                if ((revents & input_mask) != 0) {
                    checkerror = false;
                    if (!mgr->handle_read_event()) {
                        mgr->mask_del(operation::read);
                        events &= ~input_mask;
                    }
                }
                if ((revents & output_mask) != 0) {
                    checkerror = false;
                    if (!mgr->handle_write_event()) {
                        mgr->mask_del(operation::write);
                        events &= ~output_mask;
                    }
                }
                if (checkerror && ((revents & error_mask) != 0)) {
                    if (revents & POLLNVAL)
                        mgr->handle_error(sec::socket_invalid);
                    else if (revents & POLLHUP)
                        mgr->handle_error(sec::socket_disconnected);
                    else
                        mgr->handle_error(sec::socket_operation_failed);
                    mgr->mask_del(operation::read_write);
                    events = 0;
                }
                return events;
            }

            void multiplexer::add(socket_manager_ptr mgr) {
                MTL_ASSERT(index_of(mgr) == -1);
                pollfd new_entry {socket_cast<socket_id>(mgr->handle()), to_bitmask(mgr->mask()), 0};
                pollset_.emplace_back(new_entry);
                managers_.emplace_back(std::move(mgr));
            }

            void multiplexer::del(ptrdiff_t index) {
                MTL_ASSERT(index != -1);
                pollset_.erase(pollset_.begin() + index);
                managers_.erase(managers_.begin() + index);
            }

            void multiplexer::write_to_pipe(uint8_t opcode, const socket_manager_ptr &mgr) {
                MTL_ASSERT(opcode == 0 || opcode == 1 || opcode == 4);
                MTL_ASSERT(mgr != nullptr || opcode == 4);
                pollset_updater::msg_buf buf;
                if (opcode != 4)
                    mgr->ref();
                buf[0] = static_cast<byte>(opcode);
                auto value = reinterpret_cast<intptr_t>(mgr.get());
                memcpy(buf.data() + 1, &value, sizeof(intptr_t));
                variant<size_t, sec> res;
                {    // Lifetime scope of guard.
                    std::lock_guard<std::mutex> guard {write_lock_};
                    if (write_handle_ != invalid_socket)
                        res = write(write_handle_, buf);
                    else
                        res = sec::socket_invalid;
                }
                if (holds_alternative<sec>(res) && opcode != 4)
                    mgr->deref();
            }

        }    // namespace network
    }        // namespace mtl
}    // namespace nil