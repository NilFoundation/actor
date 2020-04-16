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

#include <chrono>

#include <nil/actor/stateful_actor.hpp>

#include <nil/actor/io/network/interfaces.hpp>

#include <nil/actor/after.hpp>
#include <nil/actor/event_based_actor.hpp>
#include <nil/actor/spawner_config.hpp>

#include <nil/actor/io/broker.hpp>
#include <nil/actor/io/middleman.hpp>
#include <nil/actor/io/basp_broker.hpp>
#include <nil/actor/io/system_messages.hpp>
#include <nil/actor/io/datagram_handle.hpp>

#include <nil/actor/io/basp/protocol.hpp>

#include <nil/actor/io/network/datagram_manager.hpp>
#include <nil/actor/io/network/default_multiplexer.hpp>

namespace nil {
    namespace actor {
        namespace io {

            struct connection_helper_state {
                static const char *name;
            };

            behavior connection_helper(stateful_actor<connection_helper_state> *self, actor b);
        }    // namespace io
    }        // namespace actor
}    // namespace nil
