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

#include <nil/actor/fwd.hpp>
#include <nil/actor/spawner.hpp>
#include <nil/actor/actor_control_block.hpp>

namespace nil {
    namespace actor {
        namespace openssl {

            /// @private
            expected<strong_actor_ptr> remote_actor(spawner &sys, const std::set<std::string> &mpi,
                                                    std::string host, uint16_t port);

            /// Establish a new connection to the actor at `host` on given `port`.
            /// @param host Valid hostname or IP address.
            /// @param port TCP port.
            /// @returns An `actor` to the proxy instance representing
            ///          a remote actor or an `error`.
            template<class ActorHandle = actor>
            expected<ActorHandle> remote_actor(spawner &sys, std::string host, uint16_t port) {
                detail::type_list<ActorHandle> tk;
                auto res = remote_actor(sys, sys.message_types(tk), std::move(host), port);
                if (res)
                    return actor_cast<ActorHandle>(std::move(*res));
                return std::move(res.error());
            }

        }    // namespace openssl
    }        // namespace actor
}    // namespace nil
