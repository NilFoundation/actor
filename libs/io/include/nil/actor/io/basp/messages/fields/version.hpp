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

#include <nil/actor/io/basp/version.hpp>

#include <nil/marshalling/marshalling.hpp>

namespace nil {
    namespace actor {
        namespace io {
            namespace basp {

                /// @addtogroup BASP

                /*!
                 * @brief Field containing current BASP version information.
                 * @note BASP is not backwards compatible
                 */
                template<typename TFieldBase>
                using version_field =
                    marshalling::field::int_value<TFieldBase, std::uint64_t,
                                                  marshalling::option::default_num_value<version>,
                                                  marshalling::option::valid_num_value_range<0, version>>;

                /// @}
            }    // namespace basp
        }        // namespace io
    }            // namespace actor
}    // namespace nil