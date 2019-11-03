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

#include <string>
#include <cstddef>
#include <utility>
#include <type_traits>

#include <nil/mtl/data_processor.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/raise_error.hpp>

namespace nil {
    namespace mtl {

        /// @ingroup TypeSystem
        /// Technology-independent deserialization interface.
        class deserializer : public data_processor<deserializer> {
        public:
            ~deserializer() override;

            using super = data_processor<deserializer>;

            static constexpr bool reads_state = false;
            static constexpr bool writes_state = true;

            // Boost Serialization compatibility
            using is_saving = std::false_type;
            using is_loading = std::true_type;

            explicit deserializer(actor_system &x);

            explicit deserializer(execution_unit *x = nullptr);
        };

        template<class T>
        typename std::enable_if<
            std::is_same<error, decltype(std::declval<deserializer &>().apply(std::declval<T &>()))>::value>::type
            operator&(deserializer &source, T &x) {
            auto e = source.apply(x);
            if (e)
                MTL_RAISE_ERROR("error during serialization (using operator&)");
        }

        template<class T>
        typename std::enable_if<
            std::is_same<error, decltype(std::declval<deserializer &>().apply(std::declval<T &>()))>::value,
            deserializer &>::type
            operator>>(deserializer &source, T &x) {
            source &x;
            return source;
        }

    }    // namespace mtl
}    // namespace nil
