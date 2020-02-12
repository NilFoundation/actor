//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

namespace nil {
    namespace actor {
        namespace io {
            namespace basp {

                /// @addtogroup BASP

                /// The current BASP version. Note: BASP is not backwards compatible.
                constexpr static const uint64_t version = 3;

                /// @}
            }    // namespace basp
        }        // namespace io
    }            // namespace actor
}    // namespace nil