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

#include <nil/mtl/io/basp/header.hpp>
#include <nil/mtl/io/basp/message_type.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace basp {

                /// @addtogroup BASP
                template<typename TBase>
                class client_handshake
                    : public marshalling::message_base<TBase, header_fields<TBase>, message_type_field<TBase>,
                                                       empty_word_field<TBase>, empty_byte_field<TBase>,
                                                       payload_len_field<TBase>, empty_qword_field<TBase>,
                                                       empty_qword_field<TBase>, empty_qword_field<TBase>> {
                public:
                };
                /// @}

            }    // namespace basp
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
