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

            /// Unpublishes `whom` by closing `port` or all assigned ports if `port == 0`.
            /// @param whom Actor that should be unpublished at `port`.
            /// @param port TCP port.
            template<class Handle>
            expected<void> unpublish(const Handle &whom, uint16_t port = 0) {
                if (!whom)
                    return sec::invalid_argument;
                return whom.home_system().middleman().unpublish(whom, port);
            }

        }    // namespace io
    }        // namespace actor
}    // namespace nil
