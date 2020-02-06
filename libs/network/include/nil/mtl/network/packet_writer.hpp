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

#include <vector>

#include <nil/mtl/byte.hpp>
#include <nil/mtl/detail/net_export.hpp>
#include <nil/mtl/network/fwd.hpp>
#include <nil/mtl/span.hpp>

namespace nil {
    namespace mtl {
        namespace network {

            /// Implements an interface for packet writing in application-layers.
            class MTL_NET_EXPORT packet_writer {
            public:
                using buffer_type = std::vector<byte>;

                virtual ~packet_writer();

                /// Returns a buffer for writing header information.
                virtual buffer_type next_header_buffer() = 0;

                /// Returns a buffer for writing payload content.
                virtual buffer_type next_payload_buffer() = 0;

                /// Convenience function to write a packet consisting of multiple buffers.
                /// @param buffers all buffers for the packet. The first buffer is a header
                ///                buffer, the other buffers are payload buffer.
                /// @warning this function takes ownership of `buffers`.
                template<class... Ts>
                void write_packet(Ts &... buffers) {
                    buffer_type *bufs[] = {&buffers...};
                    write_impl(make_span(bufs, sizeof...(Ts)));
                }

            protected:
                /// Implementing function for `write_packet`.
                /// @param buffers a `span` containing all buffers of a packet.
                virtual void write_impl(span<buffer_type *> buffers) = 0;
            };

        }    // namespace network
    }        // namespace mtl
}    // namespace nil