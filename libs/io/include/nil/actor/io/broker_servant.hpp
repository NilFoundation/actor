//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <nil/actor/fwd.hpp>
#include <nil/actor/mailbox_element.hpp>

#include <nil/actor/io/abstract_broker.hpp>
#include <nil/actor/io/system_messages.hpp>

namespace nil {
    namespace actor {
        namespace io {

            /// Base class for `scribe` and `doorman`.
            /// @ingroup Broker
            template<class Base, class Handle, class SysMsgType>
            class broker_servant : public Base {
            public:
                using handle_type = Handle;

                broker_servant(handle_type x) :
                    hdl_(x), value_(strong_actor_ptr {}, make_message_id(), mailbox_element::forwarding_stack {},
                                    SysMsgType {x, {}}) {
                    // nop
                }

                handle_type hdl() const {
                    return hdl_;
                }

                void halt() {
                    activity_tokens_ = none;
                    this->remove_from_loop();
                }

                void trigger() {
                    activity_tokens_ = none;
                    this->add_to_loop();
                }

                void trigger(size_t num) {
                    ACTOR_ASSERT(num > 0);
                    if (activity_tokens_)
                        *activity_tokens_ += num;
                    else
                        activity_tokens_ = num;
                    this->add_to_loop();
                }

                inline optional<size_t> activity_tokens() const {
                    return activity_tokens_;
                }

            protected:
                void detach_from(abstract_broker *ptr) override {
                    ptr->erase(hdl_);
                }

                void invoke_mailbox_element_impl(execution_unit *ctx, mailbox_element &x) {
                    auto self = this->parent();
                    auto pfac = self->proxy_registry_ptr();
                    if (pfac)
                        ctx->proxy_registry_ptr(pfac);
                    auto guard = detail::make_scope_guard([=] {
                        if (pfac)
                            ctx->proxy_registry_ptr(nullptr);
                    });
                    self->activate(ctx, x);
                }

                bool invoke_mailbox_element(execution_unit *ctx) {
                    // hold on to a strong reference while "messing" with the parent actor
                    strong_actor_ptr ptr_guard {this->parent()->ctrl()};
                    auto prev = activity_tokens_;
                    invoke_mailbox_element_impl(ctx, value_);
                    // only consume an activity token if actor did not produce them now
                    if (prev && activity_tokens_ && --(*activity_tokens_) == 0) {
                        if (this->parent()->getf(abstract_actor::is_shutting_down_flag |
                                                 abstract_actor::is_terminated_flag))
                            return false;
                        // tell broker it entered passive mode, this can result in
                        // producing, why we check the condition again afterwards
                        using passiv_t = typename std::conditional<
                            std::is_same<handle_type, connection_handle>::value, connection_passivated_msg,
                            typename std::conditional<std::is_same<handle_type, accept_handle>::value,
                                                      acceptor_passivated_msg,
                                                      datagram_servant_passivated_msg>::type>::type;
                        using tmp_t = mailbox_element_vals<passiv_t>;
                        tmp_t tmp {strong_actor_ptr {}, make_message_id(), mailbox_element::forwarding_stack {},
                                   passiv_t {hdl()}};
                        invoke_mailbox_element_impl(ctx, tmp);
                        return activity_tokens_ != size_t {0};
                    }
                    return true;
                }

                SysMsgType &msg() {
                    return value_.template get_mutable_as<SysMsgType>(0);
                }

                handle_type hdl_;
                mailbox_element_vals<SysMsgType> value_;
                optional<size_t> activity_tokens_;
            };

        }    // namespace io
    }        // namespace actor
}    // namespace nil
