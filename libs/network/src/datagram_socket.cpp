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

#include <nil/mtl/network/datagram_socket.hpp>

#include <nil/mtl/detail/net_syscall.hpp>
#include <nil/mtl/detail/socket_sys_includes.hpp>
#include <nil/mtl/logger.hpp>

namespace nil {
    namespace mtl {
        namespace network {

#ifdef MTL_WINDOWS

            error allow_connreset(datagram_socket x, bool new_value) {
                MTL_LOG_TRACE(MTL_ARG(x) << MTL_ARG(new_value));
                DWORD bytes_returned = 0;
                MTL_NET_SYSCALL("WSAIoctl", res, !=, 0,
                                WSAIoctl(x.id, _WSAIOW(IOC_VENDOR, 12), &new_value, sizeof(new_value), NULL, 0,
                                         &bytes_returned, NULL, NULL));
                return none;
            }

#else    // MTL_WINDOWS

            error allow_connreset(datagram_socket x, bool) {
                if (x == invalid_socket)
                    return sec::socket_invalid;
                // nop; SIO_UDP_CONNRESET only exists on Windows
                return none;
            }

#endif    // MTL_WINDOWS

            variant<size_t, sec> check_datagram_socket_io_res(std::make_signed<size_t>::type res) {
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