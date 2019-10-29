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

#include <cstdint>
#include <ctype.h>
#include <string>

#include <nil/mtl/atom.hpp>
#include <nil/mtl/config.hpp>
#include <nil/mtl/detail/parser/chars.hpp>
#include <nil/mtl/detail/parser/is_char.hpp>
#include <nil/mtl/detail/parser/state.hpp>
#include <nil/mtl/detail/scope_guard.hpp>
#include <nil/mtl/pec.hpp>

MTL_PUSH_UNUSED_LABEL_WARNING

#include <nil/mtl/detail/parser/fsm.hpp>

namespace nil {
    namespace mtl {
        namespace detail {
            namespace parser {

                /// Reads a number, i.e., on success produces either an `int64_t` or a
                /// `double`.
                template<class Iterator, class Sentinel, class Consumer>
                void read_atom(state<Iterator, Sentinel> &ps, Consumer &&consumer, bool accept_unquoted = false) {
                    size_t pos = 0;
                    char buf[11];
                    memset(buf, 0, sizeof(buf));
                    auto is_legal = [](char c) { return isalnum(c) || c == '_' || c == ' '; };
                    auto is_legal_no_ws = [](char c) { return isalnum(c) || c == '_'; };
                    auto append = [&](char c) {
                        if (pos == sizeof(buf) - 1)
                            return false;
                        buf[pos++] = c;
                        return true;
                    };
                    auto g = nil::mtl::detail::make_scope_guard([&] {
                        if (ps.code <= pec::trailing_character)
                            consumer.value(atom(buf));
                    });
                    // clang-format off
  start();
  state(init) {
    transition(init, " \t")
    transition(read_chars, '\'')
    epsilon_if(accept_unquoted, read_unquoted_chars, is_legal)
  }
  state(read_chars) {
    transition(done, '\'')
    transition(read_chars, is_legal, append(ch), pec::too_many_characters)
  }
  term_state(done) {
    transition(done, " \t")
  }
  term_state(read_unquoted_chars) {
    transition(read_unquoted_chars, is_legal_no_ws, append(ch), pec::too_many_characters)
  }
  fin();
                    // clang-format on
                }

            }    // namespace parser
        }        // namespace detail
    }            // namespace mtl
}    // namespace nil

#include <nil/mtl/detail/parser/fsm_undef.hpp>

MTL_POP_WARNINGS
