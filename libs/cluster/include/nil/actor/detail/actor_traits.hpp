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

#include <nil/actor/core/semaphore.hh>

namespace nil::actor {

    ///\exclude
    namespace detail {
        struct local_actor { };
    }    // namespace detail

    /// Actor attribute base class that specify that the Derived actor should be treated as a local actor
    /// \unique_name nil::actor::local_actor
    /// \requires Type `Derived` shall inherit from [nil::actor::actor]()
    /// \tparam Derived The derived [nil::actor::actor]() class for CRTP purposes
    /// \tparam ConcurrencyLimit Optional. The limit of concurrent local activations for this actor
    template<typename Derived, std::size_t ConcurrencyLimit = std::numeric_limits<std::size_t>::max()>
    struct local_actor : detail::local_actor {
        static_assert(ConcurrencyLimit > 0, "Local actor concurrency limit must be a positive integer");

        /// \exclude
        static constexpr std::size_t max_activations = ConcurrencyLimit;

        /// \exclude
        static thread_local std::size_t round_robin_counter;
    };

    /// \exclude
    template<typename Derived, std::size_t ConcurrencyLimit>
    thread_local std::size_t local_actor<Derived, ConcurrencyLimit>::round_robin_counter = 0;

    /// Actor attribute base class that specify that the Derived actor should be protected against reentrancy
    /// \unique_name nil::actor::non_reentrant_actor
    /// \requires Type `Derived` shall inherit from [nil::actor::actor]()
    /// \tparam Derived The derived actor class for CRTP purposes
    template<typename Derived>
    struct non_reentrant_actor {
        /// \exclude
        nil::actor::semaphore semaphore = nil::actor::semaphore(1);
    };

    /// Enum representing the possible kinds of [nil::actor::actor]()
    /// \unique_name nil::actor::actor_type
    enum class ActorKind { SingletonActor, LocalActor };

    /// Get the [nil::actor::actor]() type
    /// \tparam Actor The [nil::actor::actor]() type to test against
    /// \requires Type `Actor` shall inherit from [nil::actor::actor]()
    /// \returns An enum value of type [nil::actor::actor_type]()
    template<typename Actor>
    constexpr ActorKind actor_kind() {
        if constexpr (std::is_base_of_v<detail::local_actor, Actor>) {
            return ActorKind::LocalActor;
        }
        return ActorKind::SingletonActor;
    }

    /// Compile-time trait testing if the [nil::actor::actor]() type is reentrant
    /// \requires Type `Actor` shall inherit from [nil::actor::actor]()
    /// \tparam Actor The [nil::actor::actor]() type to test against
    /// \returns `true` if type `Actor` is reentrant, `false` otherwise
    template<typename Actor>
    constexpr bool is_reentrant_v = !std::is_base_of_v<non_reentrant_actor<Actor>, Actor>;

    /// Compile-time trait testing if the [nil::actor::actor]() type is reentrant
    /// \requires Type `Actor` shall inherit from [nil::actor::actor]()
    /// \tparam Actor The [nil::actor::actor]() type to test against
    /// \returns `true` if type `Actor` is reentrant, `false` otherwise
    template<typename Actor>
    constexpr bool is_reentrant = !std::is_base_of<non_reentrant_actor<Actor>, Actor>::value;

    /// Compile-time trait testing if the [nil::actor::actor]() type is local
    /// \requires Type `Actor` shall inherit from [nil::actor::actor]()
    /// \tparam Actor The actor type to test against
    /// \returns `true` if type `Actor` is local, `false` otherwise
    template<typename Actor>
    constexpr bool is_local_actor_v = std::is_base_of_v<detail::local_actor, Actor>;

    /// Compile-time trait testing if the [nil::actor::actor]() type is local
    /// \requires Type `Actor` shall inherit from [nil::actor::actor]()
    /// \tparam Actor The actor type to test against
    /// \returns `true` if type `Actor` is local, `false` otherwise
    template<typename Actor>
    constexpr bool is_local_actor = std::is_base_of<detail::local_actor, Actor>::value;

    /// Compile-time trait testing if the [nil::actor::local_actor]() type doesn't specify a concurrency limit
    /// \requires Type `Actor` shall inherit from [nil::actor::local_actor]()
    /// \tparam Actor The actor type to test against
    /// \returns `true` if `Actor` has no concurrency limit, `false` otherwise
    template<typename Actor>
    constexpr bool is_unlimited_concurrent_local_actor_v = std::is_base_of_v<local_actor<Actor>, Actor>;

    /// Compile-time trait testing if the [nil::actor::local_actor]() type doesn't specify a concurrency limit
    /// \requires Type `Actor` shall inherit from [nil::actor::local_actor]()
    /// \tparam Actor The actor type to test against
    /// \returns `true` if `Actor` has no concurrency limit, `false` otherwise
    template<typename Actor>
    constexpr bool is_unlimited_concurrent_local_actor = std::is_base_of<local_actor<Actor>, Actor>::value;

}    // namespace nil::actor
