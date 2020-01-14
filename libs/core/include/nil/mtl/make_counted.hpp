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

#include <nil/mtl/ref_counted.hpp>
#include <nil/mtl/intrusive_ptr.hpp>

#include <nil/mtl/detail/type_traits.hpp>

namespace nil {
    namespace mtl {

        /// Constructs an object of type `T` in an `intrusive_ptr`.
        /// @relates ref_counted
        /// @relates intrusive_ptr
        template<class T, class... Ts>
        intrusive_ptr<T> make_counted(Ts &&... xs) {
            return intrusive_ptr<T>(new T(std::forward<Ts>(xs)...), false);
        }
    }    // namespace mtl
}    // namespace nil
