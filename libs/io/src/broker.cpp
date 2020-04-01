//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/none.hpp>
#include <nil/actor/config.hpp>
#include <nil/actor/make_counted.hpp>

#include <nil/actor/logger.hpp>
#include <nil/actor/detail/scope_guard.hpp>

#include <nil/actor/io/broker.hpp>
#include <nil/actor/io/middleman.hpp>

#include <nil/actor/actor_registry.hpp>
#include <nil/actor/detail/sync_request_bouncer.hpp>

namespace nil {
    namespace actor {
        namespace io {

            void broker::initialize() {
                ACTOR_LOG_TRACE("");
                init_broker();
                auto bhvr = make_behavior();
                ACTOR_LOG_DEBUG_IF(!bhvr, "make_behavior() did not return a behavior:" << ACTOR_ARG2("alive", alive()));
                if (bhvr) {
                    // make_behavior() did return a behavior instead of using become()
                    ACTOR_LOG_DEBUG("make_behavior() did return a valid behavior");
                    become(std::move(bhvr));
                }
            }

            broker::broker(actor_config &cfg) : super(cfg) {
                // nop
            }

            behavior broker::make_behavior() {
                behavior res;
                if (initial_behavior_fac_) {
                    res = initial_behavior_fac_(this);
                    initial_behavior_fac_ = nullptr;
                }
                return res;
            }

        }    // namespace io
    }        // namespace actor
}    // namespace nil
