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

#pragma once

#include <nil/mtl/io/network/ip_endpoint.hpp>
#include <nil/mtl/io/network/native_socket.hpp>

namespace nil {
    namespace mtl {
        namespace policy {

            /// Policy object for wrapping default UDP operations.
            struct udp {
                /// Write a datagram containing `buf_len` bytes to `fd` addressed
                /// at the endpoint in `sa` with size `sa_len`. Returns true as long
                /// as no IO error occurs. The number of written bytes is stored in
                /// `result` and the sender is stored in `ep`.
                static bool read_datagram(size_t &result, io::network::native_socket fd, void *buf, size_t buf_len,
                                          io::network::ip_endpoint &ep);

                /// Reveice a datagram of up to `len` bytes. Larger datagrams are truncated.
                /// Up to `sender_len` bytes of the receiver address is written into
                /// `sender_addr`. Returns `true` if no IO error occurred. The number of
                /// received bytes is stored in `result` (can be 0).
                static bool write_datagram(size_t &result, io::network::native_socket fd, void *buf, size_t buf_len,
                                           const io::network::ip_endpoint &ep);

                /// Always returns `false`. Native UDP I/O event handlers only rely on the
                /// socket buffer.
                static constexpr bool must_read_more(io::network::native_socket, size_t) {
                    return false;
                }
            };

        }    // namespace scheduler_policy
    }        // namespace mtl
}    // namespace nil
