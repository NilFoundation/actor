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

namespace nil {
    namespace mtl {
        namespace detail {
            namespace parser {

                /// Returns whether `c` is a valid digit for a given base.
                template<int Base>
                bool is_digit(char c);

                template<>
                inline bool is_digit<2>(char c) {
                    return c == '0' || c == '1';
                }

                template<>
                inline bool is_digit<8>(char c) {
                    return c >= '0' && c <= '7';
                }

                template<>
                inline bool is_digit<10>(char c) {
                    return c >= '0' && c <= '9';
                }

                template<>
                inline bool is_digit<16>(char c) {
                    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
                }

            }    // namespace parser
        }        // namespace detail
    }            // namespace mtl
}    // namespace nil
