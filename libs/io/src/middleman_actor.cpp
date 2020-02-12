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

#include <nil/actor/io/middleman_actor.hpp>

#include <tuple>
#include <stdexcept>
#include <utility>

#include <nil/actor/spawner.hpp>
#include <nil/actor/spawn_options.hpp>
#include <nil/actor/spawner_config.hpp>

#include <nil/actor/io/middleman_actor_impl.hpp>

namespace nil {
    namespace actor {
        namespace io {

            middleman_actor make_middleman_actor(spawner &sys, actor db) {
                return sys.config().middleman_attach_utility_actors ?
                           sys.spawn<middleman_actor_impl, hidden>(std::move(db)) :
                           sys.spawn<middleman_actor_impl, detached + hidden>(std::move(db));
            }

        }    // namespace io
    }        // namespace actor
}    // namespace nil
