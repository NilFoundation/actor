//---------------------------------------------------------------------------//
// Copyright (c) 2011-2017 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#pragma once

#include <nil/mtl/intrusive/inbox_result.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            /// Alias for backwards compatibility.
            using enqueue_result = intrusive::inbox_result;

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
