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

#include <type_traits>

namespace nil {
    namespace mtl {

        /// Helper class to indicate that a request has been forwarded.
        template<class... Ts>
        class delegated {
            // nop
        };

        template<class... Ts>
        inline bool operator==(const delegated<Ts...> &, const delegated<Ts...> &) {
            return true;
        }

    }    // namespace mtl
}    // namespace nil
