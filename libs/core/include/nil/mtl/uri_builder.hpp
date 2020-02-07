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

#include <cstdint>
#include <string>

#include <nil/mtl/fwd.hpp>
#include <nil/mtl/uri.hpp>

namespace nil {
    namespace mtl {

        class uri_builder {
        public:
            // -- member types -----------------------------------------------------------

            /// Pointer to implementation.
            using impl_ptr = intrusive_ptr<detail::uri_impl>;

            // -- constructors, destructors, and assignment operators --------------------

            uri_builder();

            uri_builder(uri_builder &&) = default;

            uri_builder &operator=(uri_builder &&) = default;

            // -- setter -----------------------------------------------------------------

            uri_builder &scheme(std::string str);

            uri_builder &userinfo(std::string str);

            uri_builder &host(std::string str);

            uri_builder &host(ip_address addr);

            uri_builder &port(uint16_t value);

            uri_builder &path(std::string str);

            uri_builder &query(uri::query_map map);

            uri_builder &fragment(std::string str);

            // -- factory functions ------------------------------------------------------

            uri make();

        private:
            // -- member variables -------------------------------------------------------

            impl_ptr impl_;
        };

    }    // namespace mtl
}    // namespace nil