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

#include <nil/mtl/network/stream_socket.hpp>

#include <nil/mtl/byte.hpp>
#include <nil/mtl/detail/net_syscall.hpp>
#include <nil/mtl/detail/socket_sys_aliases.hpp>
#include <nil/mtl/detail/socket_sys_includes.hpp>
#include <nil/mtl/expected.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/network/socket.hpp>
#include <nil/mtl/network/socket_guard.hpp>
#include <nil/mtl/span.hpp>
#include <nil/mtl/variant.hpp>

#ifdef MTL_POSIX
#include <sys/uio.h>
#endif

namespace nil {
    namespace mtl {
        namespace network {

#ifdef MTL_WINDOWS

            constexpr int no_sigpipe_io_flag = 0;

            /**************************************************************************\
             * Based on work of others;                                               *
             * original header:                                                       *
             *                                                                        *
             * Copyright 2007, 2010 by Nathan C. Myers <ncm@cantrip.org>              *
             * Redistribution and use in source and binary forms, with or without     *
             * modification, are permitted provided that the following conditions     *
             * are met:                                                               *
             *                                                                        *
             * Redistributions of source code must retain the above copyright notice, *
             * this list of conditions and the following disclaimer.                  *
             *                                                                        *
             * Redistributions in binary form must reproduce the above copyright      *
             * notice, this list of conditions and the following disclaimer in the    *
             * documentation and/or other materials provided with the distribution.   *
             *                                                                        *
             * The name of the author must not be used to endorse or promote products *
             * derived from this software without specific prior written permission.  *
             *                                                                        *
             * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
             * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
             * LIMITED  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR *
             * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
             * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
             * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
             * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
             * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
             * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
             * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
             * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
             \**************************************************************************/
            expected<std::pair<stream_socket, stream_socket>> make_stream_socket_pair() {
                auto addrlen = static_cast<int>(sizeof(sockaddr_in));
                socket_id socks[2] = {invalid_socket_id, invalid_socket_id};
                MTL_NET_SYSCALL("socket", listener, ==, invalid_socket_id, ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
                union {
                    sockaddr_in inaddr;
                    sockaddr addr;
                } a;
                memset(&a, 0, sizeof(a));
                a.inaddr.sin_family = AF_INET;
                a.inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                a.inaddr.sin_port = 0;
                // makes sure all sockets are closed in case of an error
                auto guard = detail::make_scope_guard([&] {
                    auto e = WSAGetLastError();
                    close(socket {listener});
                    close(socket {socks[0]});
                    close(socket {socks[1]});
                    WSASetLastError(e);
                });
                // bind listener to a local port
                int reuse = 1;
                MTL_NET_SYSCALL("setsockopt", tmp1, !=, 0,
                                setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&reuse),
                                           static_cast<int>(sizeof(reuse))));
                MTL_NET_SYSCALL("bind", tmp2, !=, 0, bind(listener, &a.addr, static_cast<int>(sizeof(a.inaddr))));
                // Read the port in use: win32 getsockname may only set the port number
                // (http://msdn.microsoft.com/library/ms738543.aspx).
                memset(&a, 0, sizeof(a));
                MTL_NET_SYSCALL("getsockname", tmp3, !=, 0, getsockname(listener, &a.addr, &addrlen));
                a.inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                a.inaddr.sin_family = AF_INET;
                // set listener to listen mode
                MTL_NET_SYSCALL("listen", tmp5, !=, 0, listen(listener, 1));
                // create read-only end of the pipe
                DWORD flags = 0;
                MTL_NET_SYSCALL("WSASocketW", read_fd, ==, invalid_socket_id,
                                WSASocketW(AF_INET, SOCK_STREAM, 0, nullptr, 0, flags));
                MTL_NET_SYSCALL("connect", tmp6, !=, 0, connect(read_fd, &a.addr, static_cast<int>(sizeof(a.inaddr))));
                // get write-only end of the pipe
                MTL_NET_SYSCALL("accept", write_fd, ==, invalid_socket_id, accept(listener, nullptr, nullptr));
                close(socket {listener});
                guard.disable();
                return std::make_pair(stream_socket {read_fd}, stream_socket {write_fd});
            }

            error keepalive(stream_socket x, bool new_value) {
                MTL_LOG_TRACE(MTL_ARG(x) << MTL_ARG(new_value));
                char value = new_value ? 1 : 0;
                MTL_NET_SYSCALL("setsockopt", res, !=, 0,
                                setsockopt(x.id, SOL_SOCKET, SO_KEEPALIVE, &value, static_cast<int>(sizeof(value))));
                return none;
            }

#else    // MTL_WINDOWS

#if defined(MTL_MACOS) || defined(MTL_IOS) || defined(MTL_BSD)
            constexpr int no_sigpipe_io_flag = 0;
#else
            constexpr int no_sigpipe_io_flag = MSG_NOSIGNAL;
#endif

            expected<std::pair<stream_socket, stream_socket>> make_stream_socket_pair() {
                int sockets[2];
                MTL_NET_SYSCALL("socketpair", spair_res, !=, 0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
                return std::make_pair(stream_socket {sockets[0]}, stream_socket {sockets[1]});
            }

            error keepalive(stream_socket x, bool new_value) {
                MTL_LOG_TRACE(MTL_ARG(x) << MTL_ARG(new_value));
                int value = new_value ? 1 : 0;
                MTL_NET_SYSCALL(
                    "setsockopt", res, !=, 0,
                    setsockopt(x.id, SOL_SOCKET, SO_KEEPALIVE, &value, static_cast<unsigned>(sizeof(value))));
                return none;
            }

#endif    // MTL_WINDOWS

            error nodelay(stream_socket x, bool new_value) {
                MTL_LOG_TRACE(MTL_ARG(x) << MTL_ARG(new_value));
                int flag = new_value ? 1 : 0;
                MTL_NET_SYSCALL("setsockopt", res, !=, 0,
                                setsockopt(x.id, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<setsockopt_ptr>(&flag),
                                           static_cast<socket_size_type>(sizeof(flag))));
                return none;
            }

            variant<size_t, sec> read(stream_socket x, span<byte> buf) {
                auto res = ::recv(x.id, reinterpret_cast<socket_recv_ptr>(buf.data()), buf.size(), no_sigpipe_io_flag);
                return check_stream_socket_io_res(res);
            }

            variant<size_t, sec> write(stream_socket x, span<const byte> buf) {
                auto res = ::send(x.id, reinterpret_cast<socket_send_ptr>(buf.data()), buf.size(), no_sigpipe_io_flag);
                return check_stream_socket_io_res(res);
            }

#ifdef MTL_WINDOWS

            variant<size_t, sec> write(stream_socket x, std::initializer_list<span<const byte>> bufs) {
                MTL_ASSERT(bufs.size() < 10);
                WSABUF buf_array[10];
                auto convert = [](span<const byte> buf) {
                    auto data = const_cast<byte *>(buf.data());
                    return WSABUF {static_cast<ULONG>(buf.size()), reinterpret_cast<CHAR *>(data)};
                };
                std::transform(bufs.begin(), bufs.end(), std::begin(buf_array), convert);
                DWORD bytes_sent = 0;
                auto res = WSASend(x.id, buf_array, static_cast<DWORD>(bufs.size()), &bytes_sent, 0, nullptr, nullptr);
                if (res != 0) {
                    auto code = last_socket_error();
                    if (code == std::errc::operation_would_block || code == std::errc::resource_unavailable_try_again)
                        return sec::unavailable_or_would_block;
                    return sec::socket_operation_failed;
                }
                return static_cast<size_t>(bytes_sent);
            }

#else    // MTL_WINDOWS

            variant<size_t, sec> write(stream_socket x, std::initializer_list<span<const byte>> bufs) {
                MTL_ASSERT(bufs.size() < 10);
                iovec buf_array[10];
                auto convert = [](span<const byte> buf) { return iovec {const_cast<byte *>(buf.data()), buf.size()}; };
                std::transform(bufs.begin(), bufs.end(), std::begin(buf_array), convert);
                auto res = writev(x.id, buf_array, static_cast<int>(bufs.size()));
                return check_stream_socket_io_res(res);
            }

#endif    // MTL_WINDOWS

            variant<size_t, sec> check_stream_socket_io_res(std::make_signed<size_t>::type res) {
                if (res == 0)
                    return sec::socket_disconnected;
                if (res < 0) {
                    auto code = last_socket_error();
                    if (code == std::errc::operation_would_block || code == std::errc::resource_unavailable_try_again)
                        return sec::unavailable_or_would_block;
                    return sec::socket_operation_failed;
                }
                return static_cast<size_t>(res);
            }

        }    // namespace network
    }        // namespace mtl
}    // namespace nil