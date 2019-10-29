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

#include <vector>
#include <memory>
#include <utility>
#include <algorithm>

#include <nil/mtl/optional.hpp>

#include <nil/mtl/config.hpp>
#include <nil/mtl/behavior.hpp>
#include <nil/mtl/message_id.hpp>
#include <nil/mtl/mailbox_element.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            struct behavior_stack_mover;

            class behavior_stack {
            public:
                friend struct behavior_stack_mover;

                behavior_stack(const behavior_stack &) = delete;
                behavior_stack &operator=(const behavior_stack &) = delete;

                behavior_stack() = default;

                // erases the last (asynchronous) behavior
                void pop_back();

                void clear();

                inline bool empty() const {
                    return elements_.empty();
                }

                inline behavior &back() {
                    MTL_ASSERT(!empty());
                    return elements_.back();
                }

                inline void push_back(behavior &&what) {
                    elements_.emplace_back(std::move(what));
                }

                template<class... Ts>
                inline void emplace_back(Ts &&... xs) {
                    elements_.emplace_back(std::forward<Ts>(xs)...);
                }

                inline void cleanup() {
                    erased_elements_.clear();
                }

            private:
                std::vector<behavior> elements_;
                std::vector<behavior> erased_elements_;
            };

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
