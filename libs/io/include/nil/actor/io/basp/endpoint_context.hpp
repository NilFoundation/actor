//---------------------------------------------------------------------------//
// Copyright (c) 2011-2016 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <unordered_map>

#include <nil/actor/variant.hpp>
#include <nil/actor/response_promise.hpp>

#include <nil/actor/io/datagram_handle.hpp>
#include <nil/actor/io/connection_handle.hpp>

#include <nil/actor/io/basp/header.hpp>
#include <nil/actor/io/basp/connection_state.hpp>

namespace nil {
    namespace actor {
        namespace io {
            namespace basp {

                // stores meta information for active endpoints
                struct endpoint_context {
                    // denotes what message we expect from the remote node next
                    basp::connection_state cstate;
                    // our currently processed BASP header
                    basp::header hdr;
                    // the handle for I/O operations
                    connection_handle hdl;
                    // network-agnostic node identifier
                    node_id id;
                    // ports
                    uint16_t remote_port;
                    uint16_t local_port;
                    // pending operations to be performed after handshake completed
                    optional<response_promise> callback;
                };

            }    // namespace basp
        }        // namespace io
    }            // namespace actor
}    // namespace nil
