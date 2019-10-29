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

#include <nil/mtl/detail/append_percent_encoded.hpp>

#include <nil/mtl/config.hpp>
#include <nil/mtl/detail/append_hex.hpp>
#include <nil/mtl/string_view.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            void append_percent_encoded(std::string &str, string_view x, bool is_path) {
                for (auto ch : x)
                    switch (ch) {
                        case '/':
                            if (is_path) {
                                str += ch;
                                break;
                            }
                            MTL_ANNOTATE_FALLTHROUGH;
                        case ' ':
                        case ':':
                        case '?':
                        case '#':
                        case '[':
                        case ']':
                        case '@':
                        case '!':
                        case '$':
                        case '&':
                        case '\'':
                        case '"':
                        case '(':
                        case ')':
                        case '*':
                        case '+':
                        case ',':
                        case ';':
                        case '=':
                            str += '%';
                            append_hex(str, reinterpret_cast<uint8_t *>(&ch), 1);
                            break;
                        default:
                            str += ch;
                    }
            }

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
