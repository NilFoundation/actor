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
#include <cstddef>    // size_t
#include <type_traits>

#include <nil/mtl/data_processor.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/raise_error.hpp>

namespace nil {
    namespace mtl {

        /// @ingroup TypeSystem
        /// Technology-independent serialization interface.
        class serializer : public data_processor<serializer> {
        public:
            using super = data_processor<serializer>;

            static constexpr bool reads_state = true;
            static constexpr bool writes_state = false;

            // Boost Serialization compatibility
            using is_saving = std::true_type;
            using is_loading = std::false_type;

            explicit serializer(actor_system &sys);

            explicit serializer(execution_unit *ctx = nullptr);

            ~serializer() override;
        };

        template<class T>
        typename std::enable_if<
            std::is_same<error, decltype(std::declval<serializer &>().apply(std::declval<T &>()))>::value>::type
            operator&(serializer &sink, const T &x) {
            // implementations are required to never modify `x` while saving
            auto e = sink.apply(const_cast<T &>(x));
            if (e)
                MTL_RAISE_ERROR("error during serialization (using operator&)");
        }

        template<class T>
        typename std::enable_if<
            std::is_same<error, decltype(std::declval<serializer &>().apply(std::declval<T &>()))>::value,
            serializer &>::type
            operator<<(serializer &sink, const T &x) {
            sink &x;
            return sink;
        }
    }    // namespace mtl
}    // namespace nil
