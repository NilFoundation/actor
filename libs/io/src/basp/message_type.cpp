//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/mtl/io/basp/message_type.hpp>

#include <nil/mtl/detail/enum_to_string.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace basp {

                namespace {

                    const char *message_type_strings[] = {"server_handshake", "client_handshake", "direct_message",
                                                          "routed_message",   "proxy_creation",   "proxy_destruction",
                                                          "heartbeat"};

                }    // namespace

                std::string to_string(message_type x) {
                    return detail::enum_to_string(x, message_type_strings);
                }

            }    // namespace basp
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
