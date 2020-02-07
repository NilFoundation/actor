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

#include <string>
#include <vector>

#include <nil/mtl/detail/net_export.hpp>
#include <nil/mtl/fwd.hpp>

namespace nil {
    namespace mtl {
        namespace network {
            namespace ip {

                /// Returns all IP addresses of `host` (if any).
                std::vector<ip_address> MTL_NET_EXPORT resolve(string_view host);

                /// Returns all IP addresses of `host` (if any).
                std::vector<ip_address> MTL_NET_EXPORT resolve(ip_address host);

                /// Returns the IP addresses for a local endpoint, which is either an address,
                /// an interface name, or the string "localhost".
                std::vector<ip_address> MTL_NET_EXPORT local_addresses(string_view host);

                /// Returns the IP addresses for a local endpoint address.
                std::vector<ip_address> MTL_NET_EXPORT local_addresses(ip_address host);

                /// Returns the hostname of this device.
                std::string MTL_NET_EXPORT hostname();

            }    // namespace ip
        }        // namespace network
    }            // namespace mtl
}    // namespace nil