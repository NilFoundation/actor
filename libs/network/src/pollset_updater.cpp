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

#include <nil/actor/network/pollset_updater.hpp>

#include <cstring>

#include <nil/actor/logger.hpp>
#include <nil/actor/network/multiplexer.hpp>
#include <nil/actor/sec.hpp>
#include <nil/actor/span.hpp>
#include <nil/actor/variant.hpp>

namespace nil {
    namespace actor {
        namespace network {

            pollset_updater::pollset_updater(pipe_socket read_handle, const multiplexer_ptr &parent) :
                super(read_handle, parent), buf_size_(0) {
                mask_ = operation::read;
                nonblocking(read_handle, true);
            }

            pollset_updater::~pollset_updater() {
                // nop
            }

            bool pollset_updater::handle_read_event() {
                for (;;) {
                    auto res = read(handle(), make_span(buf_.data() + buf_size_, buf_.size() - buf_size_));
                    if (auto num_bytes = get_if<size_t>(&res)) {
                        buf_size_ += *num_bytes;
                        if (buf_.size() == buf_size_) {
                            buf_size_ = 0;
                            auto opcode = static_cast<uint8_t>(buf_[0]);
                            intptr_t value;
                            memcpy(&value, buf_.data() + 1, sizeof(intptr_t));
                            socket_manager_ptr mgr {reinterpret_cast<socket_manager *>(value), false};
                            if (auto ptr = parent_.lock()) {
                                switch (opcode) {
                                    case 0:
                                        ptr->register_reading(mgr);
                                        break;
                                    case 1:
                                        ptr->register_writing(mgr);
                                        break;
                                    case 4:
                                        ptr->shutdown();
                                        break;
                                    default:
                                        ACTOR_LOG_DEBUG("opcode not recognized: " << ACTOR_ARG(opcode));
                                        break;
                                }
                            }
                        }
                    } else {
                        return get<sec>(res) == sec::unavailable_or_would_block;
                    }
                }
            }

            bool pollset_updater::handle_write_event() {
                return false;
            }

            void pollset_updater::handle_error(sec) {
                // nop
            }

        }    // namespace network
    }        // namespace actor
}    // namespace nil