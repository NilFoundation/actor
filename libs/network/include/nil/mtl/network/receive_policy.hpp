//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2020 Nil Foundation AG
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <cstddef>
#include <string>
#include <utility>

#include <nil/mtl/config.hpp>

namespace nil {
    namespace mtl {
        namespace network {

            enum class MTL_NET_EXPORT receive_policy_flag : unsigned { at_least, at_most, exactly };

            inline std::string to_string(receive_policy_flag x) {
                return x == receive_policy_flag::at_least ? "at_least" :
                                                            (x == receive_policy_flag::at_most ? "at_most" : "exactly");
            }

            class MTL_NET_EXPORT receive_policy {
            public:
                receive_policy() = delete;

                using config = std::pair<receive_policy_flag, size_t>;

                static config at_least(size_t num_bytes) {
                    MTL_ASSERT(num_bytes > 0);
                    return {receive_policy_flag::at_least, num_bytes};
                }

                static config at_most(size_t num_bytes) {
                    MTL_ASSERT(num_bytes > 0);
                    return {receive_policy_flag::at_most, num_bytes};
                }

                static config exactly(size_t num_bytes) {
                    MTL_ASSERT(num_bytes > 0);
                    return {receive_policy_flag::exactly, num_bytes};
                }
            };

        }    // namespace network
    }        // namespace mtl
}    // namespace nil