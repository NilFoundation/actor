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

#include <string>

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                /// Identifies network IO operations, i.e., read or write.
                enum class operation { read, write, propagate_error };

                inline std::string to_string(operation op) {
                    return op == operation::read ? "read" : (op == operation::write ? "write" : "propagate_error");
                }

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
