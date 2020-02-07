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

#pragma once

#include <string>

namespace nil {
    namespace mtl {
        namespace network {
            namespace basp {

                /// @addtogroup BASP

                /// Stores the state of a connection in a `basp::application`.
                enum class connection_state {
                    /// Initial state for any connection to wait for the peer's handshake.
                    await_handshake_header,
                    /// Indicates that the header for the peer's handshake arrived and BASP
                    /// requires the payload next.
                    await_handshake_payload,
                    /// Indicates that a connection is established and this node is waiting for
                    /// the next BASP header.
                    await_header,
                    /// Indicates that this node has received a header with non-zero payload and
                    /// is waiting for the data.
                    await_payload,
                    /// Indicates that the connection is about to shut down.
                    shutdown,
                };

                /// @relates connection_state
                std::string to_string(connection_state x);

                /// @}

            }    // namespace basp
        }        // namespace network
    }            // namespace mtl
}    // namespace nil
