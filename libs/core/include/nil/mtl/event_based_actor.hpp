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

#include <type_traits>

#include <nil/mtl/fwd.hpp>
#include <nil/mtl/extend.hpp>
#include <nil/mtl/local_actor.hpp>
#include <nil/mtl/actor_marker.hpp>
#include <nil/mtl/response_handle.hpp>
#include <nil/mtl/scheduled_actor.hpp>

#include <nil/mtl/mixin/sender.hpp>
#include <nil/mtl/mixin/requester.hpp>
#include <nil/mtl/mixin/subscriber.hpp>
#include <nil/mtl/mixin/behavior_changer.hpp>

#include <nil/mtl/logger.hpp>

namespace nil {
    namespace mtl {

        template<>
        class behavior_type_of<event_based_actor> {
        public:
            using type = behavior;
        };

        /// A cooperatively scheduled, event-based actor implementation. This is the
        /// recommended base class for user-defined actors.
        /// @extends scheduled_actor
        class event_based_actor
            : public extend<scheduled_actor, event_based_actor>::with<mixin::sender, mixin::requester,
                                                                      mixin::subscriber, mixin::behavior_changer>,
              public dynamically_typed_actor_base {
        public:
            // -- member types -----------------------------------------------------------

            /// Required by `spawn` for type deduction.
            using signatures = none_t;

            /// Required by `spawn` for type deduction.
            using behavior_type = behavior;

            // -- constructors, destructors ----------------------------------------------

            explicit event_based_actor(actor_config &cfg);

            ~event_based_actor() override;

            // -- overridden functions of local_actor ------------------------------------

            void initialize() override;

        protected:
            // -- behavior management ----------------------------------------------------

            /// Returns the initial actor behavior.
            virtual behavior make_behavior();
        };

    }    // namespace mtl
}    // namespace nil
