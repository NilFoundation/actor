//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt for Boost License or
// http://opensource.org/licenses/BSD-3-Clause for BSD 3-Clause License
//---------------------------------------------------------------------------//

#pragma once

#include <set>
#include <string>
#include <cstdint>

#include <nil/mtl/fwd.hpp>
#include <nil/mtl/sec.hpp>
#include <nil/mtl/error.hpp>
#include <nil/mtl/actor_cast.hpp>
#include <nil/mtl/typed_actor.hpp>
#include <nil/mtl/actor_control_block.hpp>

namespace nil {
    namespace mtl {
        namespace openssl {

            /// @private
            expected<uint16_t> publish(actor_system &sys, const strong_actor_ptr &whom, std::set<std::string> &&sigs,
                                       uint16_t port, const char *cstr, bool ru);

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
                return publish(sys, actor_cast<strong_actor_ptr>(whom), sys.message_types(whom), port, in, reuse);
            }

        }    // namespace openssl
    }        // namespace mtl
}