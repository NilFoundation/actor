//---------------------------------------------------------------------------//
// Copyright (c) 2011-2017 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#pragma once

#include <type_traits>

#include <nil/actor/fwd.hpp>

namespace nil {
    namespace actor {
        namespace intrusive {

            /// Returns the state of a consumer from `new_round`.
            struct new_round_result {
                /// Denotes whether the consumer accepted at least one element.
                bool consumed_items : 1;
                /// Denotes whether the consumer returned `task_result::stop_all`.
                bool stop_all : 1;
            };

            constexpr bool operator==(new_round_result x, new_round_result y) {
                return x.consumed_items == y.consumed_items && x.stop_all == y.stop_all;
            }

            constexpr bool operator!=(new_round_result x, new_round_result y) {
                return !(x == y);
            }

            constexpr new_round_result make_new_round_result(bool consumed_items, bool stop_all = false) {
                return {consumed_items, stop_all};
            }

            constexpr new_round_result operator|(new_round_result x, new_round_result y) {
                return {x.consumed_items || y.consumed_items, x.stop_all || y.stop_all};
            }

        }    // namespace intrusive
    }        // namespace actor
}    // namespace nil
