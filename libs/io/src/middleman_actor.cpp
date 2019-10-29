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

#include <nil/mtl/io/middleman_actor.hpp>

#include <tuple>
#include <stdexcept>
#include <utility>

#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/spawn_options.hpp>
#include <nil/mtl/actor_system_config.hpp>

#include <nil/mtl/io/middleman_actor_impl.hpp>

namespace nil {
    namespace mtl {
        namespace io {

            middleman_actor make_middleman_actor(actor_system &sys, actor db) {
                return sys.config().middleman_attach_utility_actors ?
                           sys.spawn<middleman_actor_impl, hidden>(std::move(db)) :
                           sys.spawn<middleman_actor_impl, detached + hidden>(std::move(db));
            }

        }    // namespace io
    }        // namespace mtl
}    // namespace nil
