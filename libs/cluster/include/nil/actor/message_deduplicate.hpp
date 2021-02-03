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
#include <nil/actor/actor_ref.hpp>

#include <nil/actor/detail/arguments_vector.hpp>

namespace nil::actor {

    namespace detail {
        template<typename Actor, typename Message, typename... Args>
        class deduplicator {
            Message handler;
            arguments_vector<std::tuple<Args...>> packed;
            actor_ref<Actor> ref;

        public:
            deduplicator(Message handler, actor_ref<Actor> &ref) : handler(handler), ref(ref) {
            }

            explicit deduplicator(deduplicator const &) = delete;

            explicit deduplicator(deduplicator &&) = default;

            template<typename... TArgs>
            inline void operator()(TArgs &&...args) {
                packed.emplace_back(std::make_tuple(std::forward<TArgs>(args)...));
            }

            inline auto execute() {
                return ref.tell_packed(handler, std::move(packed));
            }
        };

        template<typename Actor, typename Message, typename Return, typename... Args>
        static constexpr auto deduplicate(actor_ref<Actor> ref, Message handler,
                                          Return (Actor::*)(Args...) const) noexcept {
            return deduplicator<Actor, Message, Args...>(handler, ref);
        }

        template<typename Actor, typename Message, typename Return, typename... Args>
        static constexpr auto deduplicate(actor_ref<Actor> ref, Message handler, Return (Actor::*)(Args...)) noexcept {
            return deduplicator<Actor, Message, Args...>(handler, ref);
        }
    }    // namespace detail

    template<typename Actor, typename Message, typename Func>
    constexpr auto deduplicate(actor_ref<Actor> ref, Message handler, Func &&func) noexcept {
        constexpr auto handlerptr = detail::vtable<Actor>::table[handler];
        return nil::actor::do_with(
            detail::deduplicate<Actor, Message>(ref, handler, handlerptr), [func = std::forward<Func>(func)](auto &d) {
                return nil::actor::futurize<void>::apply(func, d).then([&d] { return d.execute(); });
            });
    }
}    // namespace nil::actor
