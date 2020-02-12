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

#include <nil/actor/local_actor.hpp>

#include <nil/actor/detail/invoke_result_visitor.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            template<class Self>
            class default_invoke_result_visitor : public invoke_result_visitor {
            public:
                inline default_invoke_result_visitor(Self *ptr) : self_(ptr) {
                    // nop
                }

                ~default_invoke_result_visitor() override {
                    // nop
                }

                void operator()() override {
                    // nop
                }

                void operator()(error &x) override {
                    ACTOR_LOG_TRACE(ACTOR_ARG(x));
                    delegate(x);
                }

                void operator()(message &x) override {
                    ACTOR_LOG_TRACE(ACTOR_ARG(x));
                    delegate(x);
                }

                void operator()(const none_t &x) override {
                    ACTOR_LOG_TRACE(ACTOR_ARG(x));
                    delegate(x);
                }

            private:
                void deliver(response_promise &rp, error &x) {
                    ACTOR_LOG_DEBUG("report error back to requesting actor");
                    rp.deliver(std::move(x));
                }

                void deliver(response_promise &rp, message &x) {
                    ACTOR_LOG_DEBUG("respond via response_promise");
                    // suppress empty messages for asynchronous messages
                    if (x.empty() && rp.async())
                        return;
                    rp.deliver(std::move(x));
                }

                void deliver(response_promise &rp, const none_t &) {
                    error err = sec::unexpected_response;
                    deliver(rp, err);
                }

                template<class T>
                void delegate(T &x) {
                    auto rp = self_->make_response_promise();
                    if (!rp.pending()) {
                        ACTOR_LOG_DEBUG("suppress response message: invalid response promise");
                        return;
                    }
                    deliver(rp, x);
                }

                Self *self_;
            };

        }    // namespace detail
    }        // namespace actor
}    // namespace nil
