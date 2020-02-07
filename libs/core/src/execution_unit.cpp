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

#include <nil/mtl/execution_unit.hpp>

namespace nil {
    namespace mtl {

        execution_unit::execution_unit(spawner *sys) : system_(sys), proxies_(nullptr) {
            // nop
        }

        execution_unit::~execution_unit() {
            // nop
        }

    }    // namespace mtl
}    // namespace nil
