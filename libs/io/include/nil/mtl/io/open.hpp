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

#include <nil/mtl/spawner.hpp>

#include <nil/mtl/io/middleman.hpp>

namespace nil {
    namespace mtl {
        namespace io {

            /// Tries to open a port for other MTL instances to connect to.
            /// @experimental
            inline expected<uint16_t> open(spawner &sys, uint16_t port, const char *in = nullptr,
                                           bool reuse = false) {
                return sys.middleman().open(port, in, reuse);
            }

        }    // namespace io
    }        // namespace mtl
}    // namespace nil
