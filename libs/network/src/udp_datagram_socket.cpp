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

#include <nil/mtl/network/udp_datagram_socket.hpp>

#include <nil/mtl/byte.hpp>
#include <nil/mtl/detail/convert_ip_endpoint.hpp>
#include <nil/mtl/detail/net_syscall.hpp>
#include <nil/mtl/detail/socket_sys_aliases.hpp>
#include <nil/mtl/detail/socket_sys_includes.hpp>
#include <nil/mtl/expected.hpp>
#include <nil/mtl/ip_endpoint.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/network/socket_guard.hpp>

namespace nil {
    namespace mtl {
        namespace network {

#ifdef MTL_WINDOWS

            error allow_connreset(udp_datagram_socket x, bool new_value) {
                MTL_LOG_TRACE(MTL_ARG(x) << MTL_ARG(new_value));
                DWORD bytes_returned = 0;
                MTL_NET_SYSCALL("WSAIoctl", res, !=, 0,
                                WSAIoctl(x.id, _WSAIOW(IOC_VENDOR, 12), &new_value, sizeof(new_value), NULL, 0,
                                         &bytes_returned, NULL, NULL));
                return none;
            }

#else    // MTL_WINDOWS

            error allow_connreset(udp_datagram_socket x, bool) {
                if (socket_cast<net::socket>(x) == invalid_socket)
                    return sec::socket_invalid;
                // nop; SIO_UDP_CONNRESET only exists on Windows
                return none;
            }

#endif    // MTL_WINDOWS

            expected<std::pair<udp_datagram_socket, uint16_t>> make_udp_datagram_socket(ip_endpoint ep,
                                                                                        bool reuse_addr) {
                MTL_LOG_TRACE(MTL_ARG(ep));
                sockaddr_storage addr = {};
                detail::convert(ep, addr);
                MTL_NET_SYSCALL("socket", fd, ==, invalid_socket_id, ::socket(addr.ss_family, SOCK_DGRAM, 0));
                udp_datagram_socket sock {fd};
                auto sguard = make_socket_guard(sock);
                socklen_t len = (addr.ss_family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
                if (reuse_addr) {
                    int on = 1;
                    MTL_NET_SYSCALL("setsockopt", tmp1, !=, 0,
                                    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<setsockopt_ptr>(&on),
                                               static_cast<socket_size_type>(sizeof(on))));
                }
                MTL_NET_SYSCALL("bind", err, !=, 0, ::bind(sock.id, reinterpret_cast<sockaddr *>(&addr), len));
                MTL_NET_SYSCALL("getsockname", erro, !=, 0,
                                getsockname(sock.id, reinterpret_cast<sockaddr *>(&addr), &len));
                MTL_LOG_DEBUG(MTL_ARG(sock.id));
                auto port = addr.ss_family == AF_INET ? reinterpret_cast<sockaddr_in *>(&addr)->sin_port :
                                                        reinterpret_cast<sockaddr_in6 *>(&addr)->sin6_port;
                return std::make_pair(sguard.release(), port);
            }

            variant<std::pair<size_t, ip_endpoint>, sec> read(udp_datagram_socket x, span<byte> buf) {
                sockaddr_storage addr = {};
                socklen_t len = sizeof(sockaddr_storage);
                auto res = ::recvfrom(x.id, reinterpret_cast<socket_recv_ptr>(buf.data()), buf.size(), 0,
                                      reinterpret_cast<sockaddr *>(&addr), &len);
                auto ret = check_udp_datagram_socket_io_res(res);
                if (auto num_bytes = get_if<size_t>(&ret)) {
                    MTL_LOG_INFO_IF(*num_bytes == 0, "Received empty datagram");
                    MTL_LOG_WARNING_IF(*num_bytes > buf.size(),
                                       "recvfrom cut of message, only received " << MTL_ARG(buf.size()) << " of "
                                                                                 << MTL_ARG(num_bytes) << " bytes");
                    ip_endpoint ep;
                    if (auto err = detail::convert(addr, ep)) {
                        MTL_ASSERT(err.category() == error_category<sec>::value);
                        return static_cast<sec>(err.code());
                    }
                    return std::pair<size_t, ip_endpoint>(*num_bytes, ep);
                } else {
                    return get<sec>(ret);
                }
            }

            variant<size_t, sec> write(udp_datagram_socket x, span<const byte> buf, ip_endpoint ep) {
                sockaddr_storage addr = {};
                detail::convert(ep, addr);
                auto len = ep.address().embeds_v4() ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
                auto res = ::sendto(x.id, reinterpret_cast<socket_send_ptr>(buf.data()), buf.size(), 0,
                                    reinterpret_cast<sockaddr *>(&addr), len);
                auto ret = check_udp_datagram_socket_io_res(res);
                if (auto num_bytes = get_if<size_t>(&ret))
                    return *num_bytes;
                else
                    return get<sec>(ret);
            }

#ifdef MTL_WINDOWS

            variant<size_t, sec> write(udp_datagram_socket x, span<std::vector<byte> *> bufs, ip_endpoint ep) {
                MTL_ASSERT(bufs.size() < 10);
                WSABUF buf_array[10];
                auto convert = [](std::vector<byte> *buf) {
                    return WSABUF {static_cast<ULONG>(buf->size()), reinterpret_cast<CHAR *>(buf->data())};
                };
                std::transform(bufs.begin(), bufs.end(), std::begin(buf_array), convert);
                sockaddr_storage addr = {};
                detail::convert(ep, addr);
                auto len = ep.address().embeds_v4() ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
                DWORD bytes_sent = 0;
                auto res = WSASendTo(x.id, buf_array, static_cast<DWORD>(bufs.size()), &bytes_sent, 0,
                                     reinterpret_cast<sockaddr *>(&addr), len, nullptr, nullptr);
                if (res != 0) {
                    auto code = last_socket_error();
                    if (code == std::errc::operation_would_block || code == std::errc::resource_unavailable_try_again)
                        return sec::unavailable_or_would_block;
                    return sec::socket_operation_failed;
                }
                return static_cast<size_t>(bytes_sent);
            }

#else    // MTL_WINDOWS

            variant<size_t, sec> write(udp_datagram_socket x, span<std::vector<byte> *> bufs, ip_endpoint ep) {
                MTL_ASSERT(bufs.size() < 10);
                auto convert = [](std::vector<byte> *buf) { return iovec {buf->data(), buf->size()}; };
                sockaddr_storage addr = {};
                detail::convert(ep, addr);
                iovec buf_array[10];
                std::transform(bufs.begin(), bufs.end(), std::begin(buf_array), convert);
                msghdr message = {};
                memset(&message, 0, sizeof(msghdr));
                message.msg_name = &addr;
                message.msg_namelen = ep.address().embeds_v4() ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
                message.msg_iov = buf_array;
                message.msg_iovlen = bufs.size();
                auto res = sendmsg(x.id, &message, 0);
                return check_udp_datagram_socket_io_res(res);
            }

#endif    // MTL_WINDOWS

            variant<size_t, sec> check_udp_datagram_socket_io_res(std::make_signed<size_t>::type res) {
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