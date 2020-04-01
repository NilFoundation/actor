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

#include <set>
#include <string>
#include <cstdint>

#include <nil/actor/spawner.hpp>

#include <nil/actor/io/middleman.hpp>

namespace nil {
    namespace actor {
        namespace io {

            /// Tries to publish `whom` at `port` and returns either an `error` or the
            /// bound port.
            /// @param whom Actor that should be published at `port`.
            /// @param port Unused TCP port.
            /// @param in The IP address to listen to or `INADDR_ANY` if `in == nullptr`.
            /// @param reuse Create socket using `SO_REUSEADDR`.
            /// @returns The actual port the OS uses after `bind()`. If `port == 0`
            ///          the OS chooses a random high-level port.
            template<class Handle>
            expected<uint16_t> publish(const Handle &whom, uint16_t port, const char *in = nullptr,
                                       bool reuse = false) {
                if (!whom)
                    return sec::cannot_publish_invalid_actor;
                auto &sys = whom.home_system();
                return sys.middleman().publish(whom, port, in, reuse);
            }

        }    // namespace io
    }        // namespace actor
}    // namespace nil
