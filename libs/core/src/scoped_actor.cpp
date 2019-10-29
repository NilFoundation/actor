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

#include <nil/mtl/scoped_actor.hpp>

#include <nil/mtl/to_string.hpp>
#include <nil/mtl/spawn_options.hpp>
#include <nil/mtl/actor_registry.hpp>
#include <nil/mtl/scoped_execution_unit.hpp>

namespace nil {
    namespace mtl {

        namespace {

            class impl : public blocking_actor {
            public:
                impl(actor_config &cfg) : blocking_actor(cfg) {
                    // nop
                }

                void act() override {
                    MTL_LOG_ERROR("act() of scoped_actor impl called");
                }

                const char *name() const override {
                    return "scoped_actor";
                }

                void launch(execution_unit *, bool, bool hide) override {
                    MTL_PUSH_AID_FROM_PTR(this);
                    MTL_LOG_TRACE(MTL_ARG(hide));
                    MTL_ASSERT(getf(is_blocking_flag));
                    if (!hide)
                        register_at_system();
                    initialize();
                }
            };

        }    // namespace

        scoped_actor::scoped_actor(actor_system &sys, bool hide) : context_(&sys) {
            MTL_SET_LOGGER_SYS(&sys);
            actor_config cfg {&context_};
            if (hide)
                cfg.flags |= abstract_actor::is_hidden_flag;
            auto hdl = sys.spawn_impl<impl, no_spawn_options>(cfg);
            self_ = actor_cast<strong_actor_ptr>(std::move(hdl));
            prev_ = MTL_SET_AID(self_->id());
        }

        scoped_actor::~scoped_actor() {
            if (!self_)
                return;
            auto x = ptr();
            if (!x->getf(abstract_actor::is_terminated_flag))
                x->cleanup(exit_reason::normal, &context_);
            MTL_SET_AID(prev_);
        }

        blocking_actor *scoped_actor::ptr() const {
            return static_cast<blocking_actor *>(actor_cast<abstract_actor *>(self_));
        }

        std::string to_string(const scoped_actor &x) {
            return to_string(x.address());
        }

    }    // namespace mtl
}    // namespace nil
