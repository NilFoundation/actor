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

#include <nil/actor/detail/directory.hpp>

#include <nil/actor/cluster/detail/node.hpp>
#include <nil/actor/cluster/detail/message_serializer.hpp>
#include <nil/actor/cluster/detail/membership.hpp>

namespace nil::actor {
    namespace cluster {
        namespace detail {

            template<typename T>
            using ActorKey = nil::actor::detail::ActorKey<T>;

            template<typename Actor>
            struct directory {
                [[nodiscard]] static constexpr node const *hold_remote_peer(ActorKey<Actor> const &key,
                                                                            std::size_t hash) {
                    return membership::service.local().node_for_key(hash);
                }

                template<typename Ret, typename Class, typename... FArgs, typename... Args>
                static constexpr auto dispatch_message(node const &n, ActorKey<Actor> const &key,
                                                       Ret (Class::*fptr)(FArgs...) const, uint32_t id,
                                                       Args &&...args) {
                    if constexpr (std::is_same_v<Ret, void>) {
                        using Sig = nil::actor::rpc::no_wait_type(ActorKey<Actor>, FArgs...);
                        return n.rpc->make_client<Sig>(id)(*n.client, key, std::forward<Args>(args)...);
                    } else {
                        using Sig = Ret(ActorKey<Actor>, FArgs...);
                        return n.rpc->make_client<Sig>(id)(*n.client, key, std::forward<Args>(args)...);
                    }
                }

                template<typename Ret, typename Class, typename... FArgs, typename... Args>
                static constexpr auto dispatch_message(node const &n, ActorKey<Actor> const &key,
                                                       Ret (Class::*fptr)(FArgs...), uint32_t id, Args &&...args) {
                    if constexpr (std::is_same_v<Ret, void>) {
                        using Sig = nil::actor::rpc::no_wait_type(ActorKey<Actor>, FArgs...);
                        return n.rpc->make_client<Sig>(id)(*n.client, key, std::forward<Args>(args)...);
                    } else {
                        using Sig = Ret(ActorKey<Actor>, FArgs...);
                        return n.rpc->make_client<Sig>(id)(*n.client, key, std::forward<Args>(args)...);
                    }
                }

                template<typename Ret, typename Class, typename... FArgs, typename PackedArgs>
                static constexpr auto dispatch_packed_message(node const &n, ActorKey<Actor> const &key,
                                                              Ret (Class::*fptr)(FArgs...) const, uint32_t id,
                                                              PackedArgs &&args) {
                    using FutReturn = nil::actor::futurize_t<std::result_of_t<decltype(fptr)(Actor, FArgs...)>>;
                    using ReturnType =
                        typename nil::actor::detail::get0_return_type<typename FutReturn::value_type>::type;
                    if constexpr (std::is_same_v<ReturnType, void>) {
                        using Sig = nil::actor::future<>(ActorKey<Actor>, PackedArgs);
                        return n.rpc->make_client<Sig>(id | (1U << 0U))(*n.client, key, std::forward<PackedArgs>(args));
                    } else {
                        using Sig = nil::actor::future<std::vector<ReturnType>>(ActorKey<Actor>, PackedArgs);
                        return n.rpc->make_client<Sig>(id | (1U << 0U))(*n.client, key, std::forward<PackedArgs>(args));
                    }
                }

                template<typename Ret, typename Class, typename... FArgs, typename PackedArgs>
                static constexpr auto dispatch_packed_message(node const &n, ActorKey<Actor> const &key,
                                                              Ret (Class::*fptr)(FArgs...), uint32_t id,
                                                              PackedArgs &&args) {
                    using FutReturn = nil::actor::futurize_t<std::result_of_t<decltype(fptr)(Actor, FArgs...)>>;
                    using ReturnType =
                        typename nil::actor::detail::get0_return_type<typename FutReturn::value_type>::type;
                    if constexpr (std::is_same_v<ReturnType, void>) {
                        using Sig = nil::actor::future<>(ActorKey<Actor>, PackedArgs);
                        return n.rpc->make_client<Sig>(id | (1U << 0U))(*n.client, key, std::forward<PackedArgs>(args));
                    } else {
                        using Sig = nil::actor::future<std::vector<ReturnType>>(ActorKey<Actor>, PackedArgs);
                        return n.rpc->make_client<Sig>(id | (1U << 0U))(*n.client, key, std::forward<PackedArgs>(args));
                    }
                }
            };
        }    // namespace detail
    }        // namespace cluster
}    // namespace nil::actor
