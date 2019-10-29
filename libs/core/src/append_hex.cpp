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

#include <nil/mtl/detail/append_hex.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            void append_hex(std::string &result, const uint8_t *xs, size_t n) {
                if (n == 0) {
                    result += "00";
                    return;
                }
                auto tbl = "0123456789ABCDEF";
                char buf[3] = {0, 0, 0};
                for (size_t i = 0; i < n; ++i) {
                    auto c = xs[i];
                    buf[0] = tbl[c >> 4];
                    buf[1] = tbl[c & 0x0F];
                    result += buf;
                }
            }

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
