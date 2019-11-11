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

#include <nil/marshalling/marshalling.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace basp {
                /// @addtogroup BASP

                /// Protocol endianness
                typedef marshalling::option::little_endian protocol_endian;
                /// @}
            }
        }    // namespace io
    }        // namespace mtl
}