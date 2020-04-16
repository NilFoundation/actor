//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <cstdint>

#include <nil/actor/spawner.hpp>

#include <nil/actor/io/middleman.hpp>

namespace nil {
    namespace actor {
        namespace io {

            /// Tries to open a port for other ACTOR instances to connect to.
            /// @experimental
            inline expected<uint16_t> open(spawner &sys, uint16_t port, const char *in = nullptr,
                                           bool reuse = false) {
                return sys.middleman().open(port, in, reuse);
            }

        }    // namespace io
    }        // namespace actor
}    // namespace nil
