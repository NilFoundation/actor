//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt or
// http://opensource.org/licenses/BSD-3-Clause
//---------------------------------------------------------------------------//

#pragma once

#include <string>

#include <nil/mtl/detail/comparable.hpp>

namespace nil {
    namespace mtl {

        /// Represents "nothing", e.g., for clearing an `optional` by assigning `none`.
        struct none_t : detail::comparable<none_t> {
            constexpr none_t() {
                // nop
            }
            constexpr explicit operator bool() const {
                return false;
            }

            static constexpr int compare(none_t) {
                return 0;
            }
        };

        static constexpr none_t none = none_t {};

        /// @relates none_t
        inline std::string to_string(const none_t &) {
            return "none";
        }

    }    // namespace mtl
}    // namespace nil
