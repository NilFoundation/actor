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

#include <nil/mtl/detail/net_export.hpp>
#include <nil/mtl/fwd.hpp>

namespace nil {
    namespace mtl {
        namespace network {

            struct MTL_NET_EXPORT this_host {
                /// Initializes the network subsystem.
                static error startup();

                /// Release any resources of the network subsystem.
                static void cleanup();
            };

        }    // namespace network
    }        // namespace mtl
}    // namespace nil
