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

#include <string>
#include <cstdint>

#include <nil/actor/spawner.hpp>

#include <nil/actor/io/middleman.hpp>

namespace nil {
    namespace actor {
        namespace io {

            inline expected<group> remote_group(spawner &sys, const std::string &group_uri) {
                return sys.middleman().remote_group(group_uri);
            }

            inline expected<group> remote_group(spawner &sys, const std::string &group_identifier,
                                                const std::string &host, uint16_t port) {
                return sys.middleman().remote_group(group_identifier, host, port);
            }

        }    // namespace io
    }        // namespace actor
}    // namespace nil
