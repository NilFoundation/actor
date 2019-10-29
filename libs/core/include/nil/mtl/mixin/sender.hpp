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

#include <tuple>
#include <chrono>

#include <nil/mtl/actor.hpp>
#include <nil/mtl/actor_cast.hpp>
#include <nil/mtl/actor_control_block.hpp>
#include <nil/mtl/check_typed_input.hpp>
#include <nil/mtl/duration.hpp>
#include <nil/mtl/no_stages.hpp>
#include <nil/mtl/response_type.hpp>
#include <nil/mtl/response_handle.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/message.hpp>
#include <nil/mtl/message_priority.hpp>
#include <nil/mtl/response_handle.hpp>
#include <nil/mtl/response_type.hpp>

#include <nil/mtl/scheduler/abstract_coordinator.hpp>

namespace nil {
    namespace mtl {
        namespace mixin {

            /// A `sender` is an actor that supports `self->send(...)`.
            template<class Base, class Subtype>
            class sender : public Base {
            public:
                // -- member types -----------------------------------------------------------

                using extended_base = sender;

                // -- constructors, destructors, and assignment operators --------------------

                template<class... Ts>
                sender(Ts &&... xs) : Base(std::forward<Ts>(xs)...) {
                    // nop
                }

                // -- send function family ---------------------------------------------------

                /// Sends `{xs...}` as an asynchronous message to `dest` with priority `mp`.
                template<message_priority P = message_priority::normal, class Dest = actor, class... Ts>
                void send(const Dest &dest, Ts &&... xs) {
                    using detail::type_list;
                    static_assert(sizeof...(Ts) > 0, "no message to send");
                    using res_t = response_type<signatures_of_t<Dest>,
                                                detail::implicit_conversions_t<typename std::decay<Ts>::type>...>;
                    static_assert(!statically_typed<Subtype>() || statically_typed<Dest>(),
                                  "statically typed actors can only send() to other "
                                  "statically typed actors; use anon_send() or request() when "
                                  "communicating with dynamically typed actors");
                    static_assert(res_t::valid, "receiver does not accept given message");
                    static_assert(is_void_response<typename res_t::type>::value ||
                                      response_type_unbox<signatures_of_t<Subtype>, typename res_t::type>::valid,
                                  "this actor does not accept the response message");
                    if (dest)
                        dest->eq_impl(make_message_id(P), dptr()->ctrl(), dptr()->context(), std::forward<Ts>(xs)...);
                }

                /// Sends `{xs...}` as an asynchronous message to `dest` with priority `mp`.
                template<message_priority P = message_priority::normal, class... Ts>
                void send(const strong_actor_ptr &dest, Ts &&... xs) {
                    using detail::type_list;
                    static_assert(sizeof...(Ts) > 0, "no message to send");
                    static_assert(!statically_typed<Subtype>(),
                                  "statically typed actors can only send() to other "
                                  "statically typed actors; use anon_send() or request() when "
                                  "communicating with dynamically typed actors");
                    if (dest)
                        dest->get()->eq_impl(make_message_id(P), dptr()->ctrl(), dptr()->context(),
                                             std::forward<Ts>(xs)...);
                }

                template<message_priority P = message_priority::normal, class Dest = actor, class... Ts>
                void anon_send(const Dest &dest, Ts &&... xs) {
                    static_assert(sizeof...(Ts) > 0, "no message to send");
                    using token = detail::type_list<
                        typename detail::implicit_conversions<typename std::decay<Ts>::type>::type...>;
                    static_assert(response_type_unbox<signatures_of_t<Dest>, token>::valid,
                                  "receiver does not accept given message");
                    if (dest)
                        dest->eq_impl(make_message_id(P), nullptr, dptr()->context(), std::forward<Ts>(xs)...);
                }

                template<message_priority P = message_priority::normal, class Rep = int, class Period = std::ratio<1>,
                         class Dest = actor, class... Ts>
                void delayed_send(const Dest &dest, std::chrono::duration<Rep, Period> rtime, Ts &&... xs) {
                    using token = detail::type_list<
                        typename detail::implicit_conversions<typename std::decay<Ts>::type>::type...>;
                    static_assert(!statically_typed<Subtype>() || statically_typed<Dest>(),
                                  "statically typed actors are only allowed to send() to other "
                                  "statically typed actors; use anon_send() or request() when "
                                  "communicating with dynamically typed actors");
                    static_assert(response_type_unbox<signatures_of_t<Dest>, token>::valid,
                                  "receiver does not accept given message");
                    // TODO: this only checks one way, we should check for loops
                    static_assert(
                        is_void_response<typename response_type<
                                signatures_of_t<Dest>,
                                detail::implicit_conversions_t<typename std::decay<Ts>::type>...>::type>::value ||
                            response_type_unbox<
                                signatures_of_t<Subtype>,
                                typename response_type<
                                    signatures_of_t<Dest>,
                                    detail::implicit_conversions_t<typename std::decay<Ts>::type>...>::type>::valid,
                        "this actor does not accept the response message");
                    if (dest) {
                        auto &clock = dptr()->system().clock();
                        auto t = clock.now() + rtime;
                        delayed_send_impl(clock, dptr()->ctrl(), dest, P, t, std::forward<Ts>(xs)...);
                    }
                }

                template<message_priority P = message_priority::normal, class Dest = actor, class Rep = int,
                         class Period = std::ratio<1>, class... Ts>
                void delayed_anon_send(const Dest &dest, std::chrono::duration<Rep, Period> rtime, Ts &&... xs) {
                    static_assert(sizeof...(Ts) > 0, "no message to send");
                    using token = detail::type_list<
                        typename detail::implicit_conversions<typename std::decay<Ts>::type>::type...>;
                    static_assert(response_type_unbox<signatures_of_t<Dest>, token>::valid,
                                  "receiver does not accept given message");
                    if (dest) {
                        auto &clock = dptr()->system().clock();
                        auto t = clock.now() + rtime;
                        delayed_send_impl(clock, nullptr, dest, P, t, std::forward<Ts>(xs)...);
                    }
                }

            private:
                Subtype *dptr() {
                    return static_cast<Subtype *>(this);
                }

                template<class... Ts>
                static void delayed_send_impl(actor_clock &clk, strong_actor_ptr src, const group &dst,
                                              message_priority, actor_clock::time_point tout, Ts &&... xs) {
                    clk.schedule_message(tout, dst, std::move(src), make_message(std::forward<Ts>(xs)...));
                }

                template<class ActorHandle, class... Ts>
                static void delayed_send_impl(actor_clock &clk, strong_actor_ptr src, const ActorHandle &dst,
                                              message_priority prio, actor_clock::time_point tout, Ts &&... xs) {
                    clk.schedule_message(tout, actor_cast<strong_actor_ptr>(dst),
                                         make_mailbox_element(std::move(src), make_message_id(prio), no_stages,
                                                              std::forward<Ts>(xs)...));
                }
            };

        }    // namespace mixin
    }        // namespace mtl
}    // namespace nil
