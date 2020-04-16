//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/policy/udp.hpp>

#include <nil/actor/io/network/native_socket.hpp>
#include <nil/actor/logger.hpp>

#ifdef ACTOR_WINDOWS
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#endif

using nil::actor::io::network::is_error;
using nil::actor::io::network::last_socket_error;
using nil::actor::io::network::native_socket;
using nil::actor::io::network::signed_size_type;
using nil::actor::io::network::socket_error_as_string;
using nil::actor::io::network::socket_size_type;

namespace nil {
    namespace actor {
        namespace policy {

            bool udp::read_datagram(size_t &result, native_socket fd, void *buf, size_t buf_len,
                                    io::network::ip_endpoint &ep) {
                ACTOR_LOG_TRACE(ACTOR_ARG(fd));
                memset(ep.address(), 0, sizeof(sockaddr_storage));
                socket_size_type len = sizeof(sockaddr_storage);
                auto sres =
                    ::recvfrom(fd, static_cast<io::network::socket_recv_ptr>(buf), buf_len, 0, ep.address(), &len);
                if (is_error(sres, true)) {
                    // Make sure WSAGetLastError gets called immediately on Windows.
                    auto err = last_socket_error();
                    ACTOR_IGNORE_UNUSED(err);
                    ACTOR_LOG_ERROR("recvfrom failed:" << socket_error_as_string(err));
                    return false;
                }
                if (sres == 0)
                    ACTOR_LOG_INFO("Received empty datagram");
                else if (sres > static_cast<signed_size_type>(buf_len))
                    ACTOR_LOG_WARNING("recvfrom cut of message, only received " << ACTOR_ARG(buf_len) << " of "
                                                                              << ACTOR_ARG(sres) << " bytes");
                result = (sres > 0) ? static_cast<size_t>(sres) : 0;
                *ep.length() = static_cast<size_t>(len);
                return true;
            }

            bool udp::write_datagram(size_t &result, native_socket fd, void *buf, size_t buf_len,
                                     const io::network::ip_endpoint &ep) {
                ACTOR_LOG_TRACE(ACTOR_ARG(fd) << ACTOR_ARG(buf_len));
                socket_size_type len = static_cast<socket_size_type>(*ep.clength());
                auto sres =
                    ::sendto(fd, reinterpret_cast<io::network::socket_send_ptr>(buf), buf_len, 0, ep.caddress(), len);
                if (is_error(sres, true)) {
                    // Make sure WSAGetLastError gets called immediately on Windows.
                    auto err = last_socket_error();
                    ACTOR_IGNORE_UNUSED(err);
                    ACTOR_LOG_ERROR("sendto failed:" << socket_error_as_string(err));
                    return false;
                }
                result = (sres > 0) ? static_cast<size_t>(sres) : 0;
                return true;
            }

        }    // namespace policy
    }        // namespace actor
}    // namespace nil
