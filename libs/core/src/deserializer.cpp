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

#include <nil/mtl/serialization/deserializer.hpp>

#include <nil/mtl/spawner.hpp>

namespace nil {
    namespace mtl {
        deserializer::deserializer(spawner &x) noexcept : context_(x.dummy_execution_unit()) {
            // nop
        }

        deserializer::deserializer(execution_unit *x) noexcept : context_(x) {
            // nop
        }

        deserializer::~deserializer() {
            // nop
        }

        auto deserializer::apply(std::vector<bool> &x) noexcept -> result_type {
            x.clear();
            size_t size = 0;
            if (auto err = begin_sequence(size))
                return err;
            for (size_t i = 0; i < size; ++i) {
                bool tmp = false;
                if (auto err = apply(tmp))
                    return err;
                x.emplace_back(tmp);
            }
            return end_sequence();
        }
    }    // namespace mtl
}    // namespace nil
