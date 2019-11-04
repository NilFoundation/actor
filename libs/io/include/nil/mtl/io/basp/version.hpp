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

#include <nil/mtl/io/basp/endianness.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace basp {

                /// @addtogroup BASP

                /// The current BASP version. Note: BASP is not backwards compatible.
                constexpr static const uint64_t version = 3;

                /*!
                 * @brief Field containing current BASP version information.
                 * @note BASP is not backwards compatible
                 */
                typedef marshalling::field::int_value<marshalling::field_type<protocol_endian>, std::uint64_t,
                                                      marshalling::option::default_num_value<version>,
                                                      marshalling::option::valid_num_value_range<0, version>>
                    version_field;

                /// @brief Extra transport fields that every message object will contain
                typedef std::tuple<version_field> extra_transport_fields;

                /// @}
            }    // namespace basp
        }        // namespace io
    }            // namespace mtl
}    // namespace nil