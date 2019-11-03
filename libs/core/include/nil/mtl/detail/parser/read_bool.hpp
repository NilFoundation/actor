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
#include <string>

#include <nil/mtl/config.hpp>
#include <nil/mtl/detail/parser/chars.hpp>
#include <nil/mtl/detail/parser/state.hpp>
#include <nil/mtl/detail/scope_guard.hpp>
#include <nil/mtl/pec.hpp>

MTL_PUSH_UNUSED_LABEL_WARNING

#include <nil/mtl/detail/parser/fsm.hpp>

namespace nil {
    namespace mtl {
        namespace detail {
            namespace parser {

                /// Reads a boolean.
                template<class Iterator, class Sentinel, class Consumer>
                void read_bool(state<Iterator, Sentinel> &ps, Consumer &&consumer) {
                    bool res = false;
                    auto g = make_scope_guard([&] {
                        if (ps.code <= pec::trailing_character)
                            consumer.value(std::move(res));
                    });
                    start();
                    state(init) {transition(has_f, 'f')
                                     transition(has_t, 't')} state(has_f) {transition(has_fa, 'a')} state(has_fa) {
                        transition(has_fal, 'l')} state(has_fal) {transition(has_fals, 's')} state(has_fals) {
                        transition(done, 'e', res = false)} state(has_t) {transition(has_tr, 'r')} state(has_tr) {
                        transition(has_tru, 'u')} state(has_tru) {transition(done, 'e', res = true)} term_state(done) {
                        // nop
                    }
                    fin();
                }

            }    // namespace parser
        }        // namespace detail
    }            // namespace mtl
}    // namespace nil

#include <nil/mtl/detail/parser/fsm_undef.hpp>

MTL_POP_WARNINGS
