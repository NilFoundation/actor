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

#include <nil/actor/core/future.hh>
#include <nil/actor/core/reactor.hh>

#include <nil/actor/cluster/detail/distributed_directory.hpp>

namespace nil::actor {
    namespace cluster {
        namespace detail {
            template<typename Actor>
            class remote_actor_ref {
                nil::actor::detail::ActorKey<Actor> key;
                node const *loc;

            public:
                using ActorType = Actor;

                explicit constexpr remote_actor_ref(nil::actor::detail::ActorKey<Actor> k, std::size_t hash,
                                                    node const *loc) :
                    key(std::move(k)),
                    loc(loc) {
                }

                constexpr remote_actor_ref(remote_actor_ref const &) = default;

                constexpr remote_actor_ref(remote_actor_ref &&) noexcept = default;

                inline constexpr typename Actor::internal::template interface<remote_actor_ref<Actor>>
                    operator->() const {
                    return typename Actor::internal::template interface<remote_actor_ref<Actor>> {*this};
                }

                template<typename Handler, typename... Args>
                inline constexpr auto tell(Handler message, Args &&...args) const {
                    return directory<Actor>::dispatch_message(*loc, key,
                                                              nil::actor::detail::vtable<Actor>::table[message],
                                                              message.value, std::forward<Args>(args)...);
                }

                template<typename Handler, typename PackedArgs>
                constexpr auto inline tell_packed(Handler message, PackedArgs &&args) const {
                    return directory<Actor>::dispatch_packed_message(*loc, key,
                                                                     nil::actor::detail::vtable<Actor>::table[message],
                                                                     message.value, std::forward<PackedArgs>(args));
                }
            };
        }    // namespace detail
    }        // namespace cluster
}    // namespace nil::actor
