//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2018-2020 Nil Foundation AG
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <nil/actor/detail/socket_sys_includes.hpp>
#include <nil/actor/fwd.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            void convert(const ip_endpoint &src, sockaddr_storage &dst);

            error convert(const sockaddr_storage &src, ip_endpoint &dst);

        }    // namespace detail
    }        // namespace actor
}    // namespace nil
