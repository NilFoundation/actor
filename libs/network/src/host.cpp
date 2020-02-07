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

#include <nil/mtl/network/host.hpp>

#include <nil/mtl/config.hpp>
#include <nil/mtl/detail/net_syscall.hpp>
#include <nil/mtl/detail/socket_sys_includes.hpp>
#include <nil/mtl/error.hpp>
#include <nil/mtl/make_message.hpp>
#include <nil/mtl/message.hpp>
#include <nil/mtl/network/socket.hpp>
#include <nil/mtl/none.hpp>

namespace nil {
    namespace mtl {
        namespace network {

#ifdef MTL_WINDOWS

            error this_host::startup() {
                WSADATA WinsockData;
                MTL_NET_SYSCALL("WSAStartup", result, !=, 0, WSAStartup(MAKEWORD(2, 2), &WinsockData));
                return none;
            }

            void this_host::cleanup() {
                WSACleanup();
            }

#else    // MTL_WINDOWS

            error this_host::startup() {
                return none;
            }

            void this_host::cleanup() {
                // nop
            }

#endif    // MTL_WINDOWS

        }    // namespace network
    }        // namespace mtl
}    // namespace nil