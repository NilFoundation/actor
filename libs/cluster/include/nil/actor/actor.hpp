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

#include <memory>
#include <unordered_map>

#include <boost/core/noncopyable.hpp>

#include <nil/actor/core/reactor.hh>

#include <nil/actor/detail/directory.hpp>
#include <nil/actor/detail/macro.hpp>

namespace nil::actor {

    /// Base template class defining an actor
    /// \unique_name nil::actor::actor
    /// \tparam Derived The derived actor class for CRTP purposes
    /// \tparam LocalPlacementStrategy Optional. Allows to specify a custom local placement strategy. Defaults to
    /// [nil::actor::default_local_placement_strategy]() \requires `Derived` should implement actor behavior using
    /// [ULTRAMARINE_DEFINE_ACTOR]()
    template<typename Derived, typename LocalPlacementStrategy = detail::default_local_placement_strategy>
    struct actor : private boost::noncopyable {

        /// Default key type (unsigned long integer)
        /// \unique_name nil::actor::actor::KeyType
        /// See [nil::actor::actor_id]()
        using KeyType = actor_id;

        /// Default placement strategy
        using PlacementStrategy = LocalPlacementStrategy;

        /// \exclude
        static inline thread_local std::unique_ptr<detail::directory<Derived>> directory =
            std::make_unique<detail::directory<Derived>>();

        /// \effects Clears all actors of type Derived in all shards
        /// \returns A future available when all instances of this actor type have been purged
        static nil::actor::future<> clear_directory() {
            return nil::actor::smp::invoke_on_all([] {
                directory->clear();
                return nil::actor::make_ready_future();
            });
        }
    };
}    // namespace nil::actor
