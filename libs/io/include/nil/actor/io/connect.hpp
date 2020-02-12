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

#include <string>
#include <cstdint>

#include <nil/actor/node_id.hpp>
#include <nil/actor/spawner.hpp>

#include <nil/actor/io/middleman.hpp>

namespace nil {
    namespace actor {
        namespace io {

            /// Tries to connect to given node.
            /// @experimental
            inline expected<node_id> connect(spawner &sys, std::string host, uint16_t port) {
                return sys.middleman().connect(std::move(host), port);
            }

        }    // namespace io
    }        // namespace actor
}    // namespace nil
