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

namespace nil {
    namespace actor {
        namespace io {
            namespace network {

                /// Denotes the returned state of read and write operations on sockets.
                enum class rw_state {
                    /// Reports that bytes could be read or written.
                    success,
                    /// Reports that the socket is closed or faulty.
                    failure,
                    /// Reports that an empty buffer is in use and no operation was performed.
                    indeterminate
                };

            }    // namespace network
        }        // namespace io
    }            // namespace actor
}    // namespace nil
