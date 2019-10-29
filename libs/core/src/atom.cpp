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

#include <nil/mtl/atom.hpp>

#include <array>
#include <cstring>

#include <nil/mtl/string_view.hpp>

namespace nil {
    namespace mtl {

        namespace {

            /// A buffer for decoding atom values.
            using atom_value_buf = std::array<char, 11>;

            size_t decode(atom_value_buf &buf, atom_value what) {
                size_t pos = 0;
                auto x = static_cast<uint64_t>(what);
                // Don't read characters before we found the leading 0xF.
                bool read_chars = ((x & 0xF000000000000000) >> 60) == 0xF;
                uint64_t mask = 0x0FC0000000000000;
                for (int bitshift = 54; bitshift >= 0; bitshift -= 6, mask >>= 6) {
                    if (read_chars)
                        buf[pos++] = detail::decoding_table[(x & mask) >> bitshift];
                    else if (((x & mask) >> bitshift) == 0xF)
                        read_chars = true;
                }
                buf[pos] = '\0';
                return pos;
            }

        }    // namespace

        atom_value to_lowercase(atom_value x) {
            atom_value_buf buf;
            decode(buf, x);
            for (auto ch = buf.data(); *ch != '\0'; ++ch)
                *ch = static_cast<char>(tolower(*ch));
            return static_cast<atom_value>(detail::atom_val(buf.data()));
        }

        atom_value atom_from_string(string_view x) {
            if (x.size() > 10)
                return atom("");
            atom_value_buf buf;
            memcpy(buf.data(), x.data(), x.size());
            buf[x.size()] = '\0';
            return static_cast<atom_value>(detail::atom_val(buf.data()));
        }

        std::string to_string(const atom_value &x) {
            atom_value_buf str;
            auto len = decode(str, x);
            return std::string(str.begin(), str.begin() + len);
        }

        int compare(atom_value x, atom_value y) {
            return memcmp(&x, &y, sizeof(atom_value));
        }

    }    // namespace mtl
}    // namespace nil
