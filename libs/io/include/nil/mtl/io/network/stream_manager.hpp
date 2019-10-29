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

#include <cstddef>

#include <nil/mtl/io/network/manager.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                /// A stream manager configures an IO stream and provides callbacks
                /// for incoming data as well as for error handling.
                class stream_manager : public manager {
                public:
                    ~stream_manager() override;

                    /// Called by the underlying I/O device whenever it received data.
                    /// @returns `true` if the manager accepts further reads, otherwise `false`.
                    virtual bool consume(execution_unit *ctx, const void *buf, size_t bsize) = 0;

                    /// Called by the underlying I/O device whenever it sent data.
                    virtual void data_transferred(execution_unit *ctx, size_t num_bytes, size_t remaining_bytes) = 0;

                    /// Get the port of the underlying I/O device.
                    virtual uint16_t port() const = 0;
                };

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
