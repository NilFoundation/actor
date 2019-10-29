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

#include <nil/mtl/io/network/manager.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                /// An acceptor manager configures an acceptor and provides
                /// callbacks for incoming connections as well as for error handling.
                class acceptor_manager : public manager {
                public:
                    ~acceptor_manager() override;

                    /// Called by the underlying I/O device to indicate that
                    /// a new connection is awaiting acceptance.
                    /// @returns `true` if the manager accepts further connections,
                    ///          otherwise `false`.
                    virtual bool new_connection() = 0;

                    /// Get the port of the underlying I/O device.
                    virtual uint16_t port() const = 0;
                };

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
