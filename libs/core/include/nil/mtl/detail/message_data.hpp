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
#include <iterator>
#include <typeinfo>

#include <nil/mtl/config.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/intrusive_cow_ptr.hpp>
#include <nil/mtl/intrusive_ptr.hpp>
#include <nil/mtl/ref_counted.hpp>
#include <nil/mtl/type_erased_tuple.hpp>

#include <nil/mtl/detail/type_list.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            class message_data : public ref_counted, public type_erased_tuple {
            public:
                // -- nested types -----------------------------------------------------------

                using cow_ptr = intrusive_cow_ptr<message_data>;

                // -- constructors, destructors, and assignment operators --------------------

                message_data() = default;
                message_data(const message_data &) = default;

                ~message_data() override;

                // -- pure virtual observers -------------------------------------------------

                virtual message_data *copy() const = 0;

                // -- observers --------------------------------------------------------------

                using type_erased_tuple::copy;

                bool shared() const noexcept override;
            };

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
