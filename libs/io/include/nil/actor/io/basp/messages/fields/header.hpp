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

#include <string>
#include <cstdint>

#include <nil/actor/io/basp/message_type.hpp>

#include <nil/actor/io/basp/messages/fields/payload_len.hpp>
#include <nil/actor/io/basp/messages/fields/actor.hpp>

namespace nil {
    namespace actor {
        namespace io {
            namespace basp {

                /// @addtogroup BASP

                template<typename FieldBaseType>
                using padding1_field = marshalling::field::int_value<FieldBaseType, std::uint8_t>;

                template<typename FieldBaseType>
                using padding2_field = marshalling::field::int_value<FieldBaseType, std::uint8_t>;

                template<typename FieldBaseType>
                using flags_field = marshalling::field::int_value<FieldBaseType, std::uint8_t>;

                template<typename FieldBaseType>
                using operation_data_field = marshalling::field::int_value<FieldBaseType, std::uint64_t>;

                template<typename TFieldBase>
                using header_fields =
                    marshalling::field::bundle<padding1_field<TFieldBase>, padding2_field<TFieldBase>,
                                               flags_field<TFieldBase>, payload_len_field<TFieldBase>,
                                               operation_data_field<TFieldBase>, actor_id_field<TFieldBase>,
                                               dflt_actor_id_field<TFieldBase>>;

                /// @}

            }    // namespace basp
        }        // namespace io
    }            // namespace actor
}    // namespace nil
