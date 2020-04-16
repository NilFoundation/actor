//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <nil/marshalling/marshalling.hpp>

namespace nil {
    namespace actor {
        namespace io {
            namespace basp {
                /// @addtogroup BASP

                template<typename TFieldBase>
                using empty_byte_field =
                    marshalling::field::no_value<marshalling::field::int_value<TFieldBase, std::uint8_t>>;

                template<typename TFieldBase>
                using empty_word_field =
                    marshalling::field::no_value<marshalling::field::int_value<TFieldBase, std::uint16_t>>;

                template<typename TFieldBase>
                using empty_dword_field =
                    marshalling::field::no_value<marshalling::field::int_value<TFieldBase, std::uint32_t>>;

                template<typename TFieldBase>
                using empty_qword_field =
                    marshalling::field::no_value<marshalling::field::int_value<TFieldBase, std::uint64_t>>;

                /// @}
            }    // namespace basp
        }        // namespace io
    }            // namespace actor
}    // namespace nil