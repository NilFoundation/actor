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

#include <string>
#include <type_traits>

#include <nil/mtl/detail/type_traits.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            void append_hex(std::string &result, const uint8_t *xs, size_t n);

            template<class T>
            enable_if_t<has_data_member<T>::value> append_hex(std::string &result, const T &x) {
                return append_hex(result, reinterpret_cast<const uint8_t *>(x.data()), x.size());
            }

            template<class T>
            enable_if_t<std::is_integral<T>::value> append_hex(std::string &result, const T &x) {
                return append_hex(result, reinterpret_cast<const uint8_t *>(&x), sizeof(T));
            }

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
