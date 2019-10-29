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

#include <cstdint>

#include <nil/mtl/actor_system.hpp>

#include <nil/mtl/io/middleman.hpp>

namespace nil {
    namespace mtl {
        namespace io {

            /// Closes port `port` regardless of whether an actor is published to it.
            inline expected<void> close(actor_system &sys, uint16_t port) {
                return sys.middleman().close(port);
            }

        }    // namespace io
    }        // namespace mtl
}    // namespace nil
