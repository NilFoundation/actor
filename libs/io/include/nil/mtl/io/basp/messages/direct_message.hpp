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

#include <nil/mtl/io/basp/messages/fields/endian.hpp>
#include <nil/mtl/io/basp/messages/fields/empty.hpp>
#include <nil/mtl/io/basp/messages/fields/version.hpp>
#include <nil/mtl/io/basp/messages/fields/message_id.hpp>
#include <nil/mtl/io/basp/messages/fields/payload_len.hpp>
#include <nil/mtl/io/basp/messages/fields/actor_id.hpp>
#include <nil/mtl/io/basp/messages/fields/message_type.hpp>
#include <nil/mtl/io/basp/messages/fields/header.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace basp {

                /// @addtogroup BASP
                template<typename TBase>
                class direct_messsage
                    : public marshalling::message_base<
                          TBase, header_fields<TBase>, message_type_field<TBase>, empty_word_field<TBase>,
                          marshalling::field::int_value<TBase, std::uint8_t>, payload_len_field<TBase>,
                          message_id_field<TBase>, actor_id_field<TBase>, dflt_actor_id_field<TBase>> {
                public:
                };
                /// @}

            }    // namespace basp
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
