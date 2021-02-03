//---------------------------------------------------------------------------//
// Copyright (c) 2018-2021 Mikhail Komarov <nemo@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#pragma once

#include <unordered_map>
#include <functional>

#include <boost/noncopyable.hpp>

#include <nil/actor/actor_ref.hpp>
#include <nil/actor/detail/arguments_vector.hpp>

#include "message_serializer.hpp"

namespace nil::actor {
    namespace cluster {
        namespace impl {

            struct static_init : public boost::noncopyable {
                explicit static_init(void (*func)()) {
                    func();
                }
            };

            inline auto &message_handler_registry() {
                static std::unordered_map<uint32_t, std::function<void(rpc_proto *)>> init_handlers = {};
                return init_handlers;
            }

            template<typename Actor, typename ActorKey, typename Ret, typename Class, typename... Args,
                     typename Handler>
            static constexpr void __attribute__((used))
            register_remote_endpoint(Ret (Class::*fptr)(Args...), Handler message) {
                auto reg = [fptr, message](auto *rpc) {
                    rpc->register_handler(message.value, [message](ActorKey key, Args... args) {
                        return nil::actor::get<Actor>(std::forward<ActorKey>(key))
                            .tell(message, std::forward<Args>(args)...);
                    });

                    // packed version
                    uint32_t packed_message_id = message.value | (1U << 0U);
                    using ArgPack = nil::actor::detail::arguments_vector<std::tuple<Args...>>;
                    rpc->register_handler(packed_message_id, [message](ActorKey key, ArgPack args) {
                        auto actor = nil::actor::get<Actor>(std::forward<ActorKey>(key));
                        return actor.tell_packed(message, std::forward<ArgPack>(args));
                    });
                };
                message_handler_registry().insert({message.value, reg});
            }

            template<typename Actor, typename ActorKey, typename Ret, typename Class, typename... Args,
                     typename Handler>
            static constexpr void __attribute__((used))
            register_remote_endpoint(Ret (Class::*fptr)(Args...) const, Handler message) {
                auto reg = [fptr, message](auto *rpc) {
                    rpc->register_handler(message.value, [message](ActorKey key, Args... args) {
                        return nil::actor::get<Actor>(std::forward<ActorKey>(key))
                            .tell(message, std::forward<Args>(args)...);
                    });

                    // packed version
                    uint32_t packed_message_id = message.value | (1U << 0U);
                    using ArgPack = nil::actor::detail::arguments_vector<std::tuple<Args...>>;
                    rpc->register_handler(packed_message_id, [message](ActorKey key, ArgPack args) {
                        auto actor = nil::actor::get<Actor>(std::forward<ActorKey>(key));
                        return actor.tell_packed(message, std::forward<ArgPack>(args));
                    });
                };
                message_handler_registry().insert({message.value, reg});
            }
        }    // namespace impl
    }        // namespace cluster
}    // namespace nil::actor
