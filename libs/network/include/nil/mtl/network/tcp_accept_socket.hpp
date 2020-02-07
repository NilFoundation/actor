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

#include <nil/mtl/detail/net_export.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/network/fwd.hpp>
#include <nil/mtl/network/network_socket.hpp>
#include <nil/mtl/uri.hpp>

namespace nil {
    namespace mtl {
        namespace network {

            /// Represents a TCP acceptor in listening mode.
            struct MTL_NET_EXPORT tcp_accept_socket : network_socket {
                using super = network_socket;

                using super::super;
            };

            /// Creates a new TCP socket to accept connections on a given port.
            /// @param node The endpoint to listen on and the filter for incoming addresses.
            /// Passing the address `0.0.0.0` will accept incoming connection from any host.
            /// Passing port 0 lets the OS choose the port.
            /// @relates tcp_accept_socket
            expected<tcp_accept_socket> MTL_NET_EXPORT make_tcp_accept_socket(ip_endpoint node,
                                                                              bool reuse_addr = false);

            /// Creates a new TCP socket to accept connections on a given port.
            /// @param node The endpoint to listen on and the filter for incoming addresses.
            /// Passing the address `0.0.0.0` will accept incoming connection from any host.
            /// Passing port 0 lets the OS choose the port.
            /// @param reuse_addr Optionally sets the SO_REUSEADDR option on the socket.
            /// @relates tcp_accept_socket
            expected<tcp_accept_socket> MTL_NET_EXPORT make_tcp_accept_socket(const uri::authority_type &node,
                                                                              bool reuse_addr = false);

            /// Accepts a connection on `x`.
            /// @param x Listening endpoint.
            /// @returns The socket that handles the accepted connection on success, an
            /// error otherwise.
            /// @relates tcp_accept_socket
            expected<tcp_stream_socket> MTL_NET_EXPORT accept(tcp_accept_socket x);

        }    // namespace network
    }        // namespace mtl
}    // namespace nil
