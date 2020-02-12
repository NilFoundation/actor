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

#include <cstdint>

#include <nil/actor/sec.hpp>
#include <nil/actor/error.hpp>
#include <nil/actor/expected.hpp>
#include <nil/actor/actor_cast.hpp>
#include <nil/actor/typed_actor.hpp>
#include <nil/actor/function_view.hpp>
#include <nil/actor/actor_control_block.hpp>

#include <nil/actor/openssl/manager.hpp>

namespace nil {
    namespace actor {
        namespace openssl {

            /// Unpublishes `whom` by closing `port` or all assigned ports if `port == 0`.
            /// @param whom Actor that should be unpublished at `port`.
            /// @param port TCP port.
            template<class Handle>
            expected<void> unpublish(const Handle &whom, uint16_t port = 0) {
                if (!whom)
                    return sec::invalid_argument;
                auto &sys = whom.home_system();
                auto f = make_function_view(sys.openssl_manager().actor_handle());
                return f(unpublish_atom::value, port);
            }

        }    // namespace openssl
    }        // namespace actor
}