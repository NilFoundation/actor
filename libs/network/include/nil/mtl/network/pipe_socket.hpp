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

#include <cstddef>
#include <system_error>
#include <utility>

#include <nil/mtl/detail/net_export.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/network/socket.hpp>
#include <nil/mtl/network/socket_id.hpp>

namespace nil {
    namespace mtl {
        namespace network {

            /// A unidirectional communication endpoint for inter-process communication.
            struct MTL_NET_EXPORT pipe_socket : socket {
                using super = socket;

                using super::super;
            };

            /// Creates two connected sockets. The first socket is the read handle and the
            /// second socket is the write handle.
            /// @relates pipe_socket
            expected<std::pair<pipe_socket, pipe_socket>> MTL_NET_EXPORT make_pipe();

            /// Transmits data from `x` to its peer.
            /// @param x Connected endpoint.
            /// @param buf Points to the message to send.
            /// @param buf_size Specifies the size of the buffer in bytes.
            /// @returns The number of written bytes on success, otherwise an error code.
            /// @relates pipe_socket
            variant<size_t, sec> MTL_NET_EXPORT write(pipe_socket x, span<const byte> buf);

            /// Receives data from `x`.
            /// @param x Connected endpoint.
            /// @param buf Points to destination buffer.
            /// @param buf_size Specifies the maximum size of the buffer in bytes.
            /// @returns The number of received bytes on success, otherwise an error code.
            /// @relates pipe_socket
            variant<size_t, sec> MTL_NET_EXPORT read(pipe_socket x, span<byte>);

            /// Converts the result from I/O operation on a ::pipe_socket to either an
            /// error code or a non-zero positive integer.
            /// @relates pipe_socket
            variant<size_t, sec> MTL_NET_EXPORT check_pipe_socket_io_res(std::make_signed<size_t>::type res);

        }    // namespace network
    }        // namespace mtl
}    // namespace nil