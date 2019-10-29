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

#include <nil/mtl/intrusive_cow_ptr.hpp>

namespace nil {
    namespace mtl {

        /// Constructs an object of type `T` in an `intrusive_cow_ptr`.
        /// @relates ref_counted
        /// @relates intrusive_cow_ptr
        template<class T, class... Ts>
        intrusive_cow_ptr<T> make_copy_on_write(Ts &&... xs) {
            return intrusive_cow_ptr<T>(new T(std::forward<Ts>(xs)...), false);
        }

    }    // namespace mtl
}    // namespace nil
