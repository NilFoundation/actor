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

#include <stdexcept>

#include <nil/mtl/detail/net_export.hpp>
#include <nil/mtl/error.hpp>
#include <nil/mtl/network/host.hpp>

namespace {

    struct MTL_NET_EXPORT host_fixture {
        host_fixture() {
            if (auto err = caf::net::this_host::startup())
                throw std::logic_error("this_host::startup failed");
        }

        ~host_fixture() {
            caf::net::this_host::cleanup();
        }
    };

}    // namespace
