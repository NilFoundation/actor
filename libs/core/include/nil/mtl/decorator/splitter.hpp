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

#include <vector>

#include <nil/mtl/actor_addr.hpp>
#include <nil/mtl/mailbox_element.hpp>
#include <nil/mtl/monitorable_actor.hpp>

namespace nil {
    namespace mtl {
        namespace decorator {

            /// An actor decorator implementing "dot operator"-like compositions,
            /// i.e., `f.g(x) = f(g(x))`. Composed actors are hidden actors.
            /// A composed actor exits when either of its constituent actors exits;
            /// Constituent actors have no dependency on the composed actor
            /// by default, and exit of a composed actor has no effect on its
            /// constituent actors. A composed actor is hosted on the same actor
            /// system and node as `g`, the first actor on the forwarding chain.
            class splitter : public monitorable_actor {
            public:
                using message_types_set = std::set<std::string>;

                splitter(std::vector<strong_actor_ptr> workers, message_types_set msg_types);

                // non-system messages are processed and then forwarded;
                // system messages are handled and consumed on the spot;
                // in either case, the processing is done synchronously
                void enqueue(mailbox_element_ptr what, execution_unit *context) override;

                message_types_set message_types() const override;

            private:
                const size_t num_workers;
                std::vector<strong_actor_ptr> workers_;
                message_types_set msg_types_;
            };

        }    // namespace decorator
    }        // namespace mtl
}    // namespace nil
