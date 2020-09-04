//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/policy/tcp.hpp>

#include <cstring>

#include <nil/actor/io/network/native_socket.hpp>
#include <nil/actor/logger.hpp>

#ifdef BOOST_OS_WINDOWS_AVAILABLE
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

using nil::actor::io::network::is_error;
using nil::actor::io::network::last_socket_error;
using nil::actor::io::network::native_socket;
using nil::actor::io::network::no_sigpipe_io_flag;
using nil::actor::io::network::rw_state;
using nil::actor::io::network::socket_error_as_string;
using nil::actor::io::network::socket_size_type;

namespace nil {
    namespace actor {
        namespace policy {

            rw_state tcp::read_some(size_t &result, native_socket fd, void *buf, size_t len) {
                ACTOR_LOG_TRACE(ACTOR_ARG(fd) << ACTOR_ARG(len));
                auto sres = ::recv(fd, reinterpret_cast<io::network::socket_recv_ptr>(buf), len, no_sigpipe_io_flag);
                if (is_error(sres, true)) {
                    // Make sure WSAGetLastError gets called immediately on Windows.
                    auto err = last_socket_error();
                    ACTOR_IGNORE_UNUSED(err);
                    ACTOR_LOG_ERROR("recv failed:" << socket_error_as_string(err));
                    return rw_state::failure;
                } else if (sres == 0) {
                    // recv returns 0  when the peer has performed an orderly shutdown
                    ACTOR_LOG_DEBUG("peer performed orderly shutdown" << ACTOR_ARG(fd));
                    return rw_state::failure;
                }
                ACTOR_LOG_DEBUG(ACTOR_ARG(len) << ACTOR_ARG(fd) << ACTOR_ARG(sres));
                result = (sres > 0) ? static_cast<size_t>(sres) : 0;
                return rw_state::success;
            }

            rw_state tcp::write_some(size_t &result, native_socket fd, const void *buf, size_t len) {
                ACTOR_LOG_TRACE(ACTOR_ARG(fd) << ACTOR_ARG(len));
                auto sres = ::send(fd, reinterpret_cast<io::network::socket_send_ptr>(buf), len, no_sigpipe_io_flag);
                if (is_error(sres, true)) {
                    // Make sure WSAGetLastError gets called immediately on Windows.
                    auto err = last_socket_error();
                    ACTOR_IGNORE_UNUSED(err);
                    ACTOR_LOG_ERROR("send failed:" << socket_error_as_string(err));
                    return rw_state::failure;
                }
                ACTOR_LOG_DEBUG(ACTOR_ARG(len) << ACTOR_ARG(fd) << ACTOR_ARG(sres));
                result = (sres > 0) ? static_cast<size_t>(sres) : 0;
                return rw_state::success;
            }

            bool tcp::try_accept(native_socket &result, native_socket fd) {
                using namespace io::network;
                ACTOR_LOG_TRACE(ACTOR_ARG(fd));
                sockaddr_storage addr;
                std::memset(&addr, 0, sizeof(addr));
                socket_size_type addrlen = sizeof(addr);
                result = ::accept(fd, reinterpret_cast<sockaddr *>(&addr), &addrlen);
                // note accept4 is better to avoid races in setting CLOEXEC (but not posix)
                if (result == invalid_native_socket) {
                    auto err = last_socket_error();
                    if (!would_block_or_temporarily_unavailable(err)) {
                        ACTOR_LOG_ERROR("accept failed:" << socket_error_as_string(err));
                        return false;
                    }
                }
                child_process_inherit(result, false);
                ACTOR_LOG_DEBUG(ACTOR_ARG(fd) << ACTOR_ARG(result));
                return true;
            }

        }    // namespace policy
    }        // namespace actor
}    // namespace nil
