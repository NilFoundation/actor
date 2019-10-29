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

#include <chrono>

#include <nil/mtl/stateful_actor.hpp>

#include <nil/mtl/io/network/interfaces.hpp>

#include <nil/mtl/after.hpp>
#include <nil/mtl/event_based_actor.hpp>
#include <nil/mtl/actor_system_config.hpp>

#include <nil/mtl/io/broker.hpp>
#include <nil/mtl/io/middleman.hpp>
#include <nil/mtl/io/basp_broker.hpp>
#include <nil/mtl/io/system_messages.hpp>
#include <nil/mtl/io/datagram_handle.hpp>

#include <nil/mtl/io/basp/all.hpp>

#include <nil/mtl/io/network/datagram_manager.hpp>
#include <nil/mtl/io/network/default_multiplexer.hpp>

namespace nil {
    namespace mtl {
        namespace io {

            struct connection_helper_state {
                static const char *name;
            };

            behavior connection_helper(stateful_actor<connection_helper_state> *self, actor b);
        }    // namespace io
    }        // namespace mtl
}    // namespace nil
