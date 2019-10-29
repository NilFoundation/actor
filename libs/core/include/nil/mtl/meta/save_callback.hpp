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

#include <nil/mtl/meta/annotation.hpp>

namespace nil {
    namespace mtl {
        namespace meta {

            template<class F>
            struct save_callback_t : annotation {
                save_callback_t(F &&f) : fun(f) {
                    // nop
                }

                save_callback_t(save_callback_t &&) = default;

                save_callback_t(const save_callback_t &) = default;

                F fun;
            };

            /// Returns an annotation that allows inspectors to call
            /// user-defined code after performing save operations.
            template<class F>
            save_callback_t<F> save_callback(F fun) {
                return {std::move(fun)};
            }

        }    // namespace meta
    }        // namespace mtl
}    // namespace nil
