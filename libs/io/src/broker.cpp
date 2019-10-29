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

#include <nil/mtl/none.hpp>
#include <nil/mtl/config.hpp>
#include <nil/mtl/make_counted.hpp>

#include <nil/mtl/logger.hpp>
#include <nil/mtl/detail/scope_guard.hpp>

#include <nil/mtl/io/broker.hpp>
#include <nil/mtl/io/middleman.hpp>

#include <nil/mtl/actor_registry.hpp>
#include <nil/mtl/detail/sync_request_bouncer.hpp>

namespace nil {
    namespace mtl {
        namespace io {

            void broker::initialize() {
                MTL_LOG_TRACE("");
                init_broker();
                auto bhvr = make_behavior();
                MTL_LOG_DEBUG_IF(!bhvr, "make_behavior() did not return a behavior:" << MTL_ARG2("alive", alive()));
                if (bhvr) {
                    // make_behavior() did return a behavior instead of using become()
                    MTL_LOG_DEBUG("make_behavior() did return a valid behavior");
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
    }        // namespace mtl
}    // namespace nil
