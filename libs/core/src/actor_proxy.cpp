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

#include <utility>

#include <nil/mtl/locks.hpp>

#include <nil/mtl/atom.hpp>
#include <nil/mtl/message.hpp>
#include <nil/mtl/actor_proxy.hpp>
#include <nil/mtl/exit_reason.hpp>

namespace nil {
    namespace mtl {

        actor_proxy::actor_proxy(actor_config &cfg) : monitorable_actor(cfg) {
            // nop
        }

        actor_proxy::~actor_proxy() {
            // nop
        }

    }    // namespace mtl
}    // namespace nil
