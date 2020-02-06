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
#include <system_error>
#include <type_traits>

#include <nil/mtl/config.hpp>
#include <nil/mtl/detail/comparable.hpp>
#include <nil/mtl/detail/net_export.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/network/socket_id.hpp>

namespace nil {
    namespace mtl {
        namespace network {

            /// An internal endpoint for sending or receiving data. Can be either a
            /// ::network_socket, ::pipe_socket, ::stream_socket, or ::datagram_socket.
            struct MTL_NET_EXPORT socket : detail::comparable<socket> {
                socket_id id;

                constexpr socket() noexcept : id(invalid_socket_id) {
                    // nop
                }

                constexpr explicit socket(socket_id id) noexcept : id(id) {
                    // nop
                }

                constexpr socket(const socket &other) noexcept = default;

                socket &operator=(const socket &other) noexcept = default;

                constexpr signed_socket_id compare(socket other) const noexcept {
                    return static_cast<signed_socket_id>(id) - static_cast<signed_socket_id>(other.id);
                }
            };

            /// @relates socket
            template<class Inspector>
            typename Inspector::result_type MTL_NET_EXPORT inspect(Inspector &f, socket &x) {
                return f(x.id);
            }

            /// Denotes the invalid socket.
            constexpr auto invalid_socket = socket {invalid_socket_id};

            /// Converts between different socket types.
            template<class To, class From>
            To MTL_NET_EXPORT socket_cast(From x) {
                return To {x.id};
            }

            /// Close socket `x`.
            /// @relates socket
            void MTL_NET_EXPORT close(socket x);

            /// Returns the last socket error in this thread as an integer.
            /// @relates socket
            std::errc MTL_NET_EXPORT last_socket_error();

            /// Returns the last socket error as human-readable string.
            /// @relates socket
            std::string MTL_NET_EXPORT last_socket_error_as_string();

            /// Sets x to be inherited by child processes if `new_value == true`
            /// or not if `new_value == false`.  Not implemented on Windows.
            /// @relates socket
            error MTL_NET_EXPORT child_process_inherit(socket x, bool new_value);

            /// Enables or disables nonblocking I/O on `x`.
            /// @relates socket
            error MTL_NET_EXPORT nonblocking(socket x, bool new_value);

        }    // namespace network
    }        // namespace mtl
}    // namespace nil
