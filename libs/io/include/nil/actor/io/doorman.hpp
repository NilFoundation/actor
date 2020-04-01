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

#include <cstddef>
#include <cstdint>

#include <nil/actor/message.hpp>
#include <nil/actor/mailbox_element.hpp>

#include <nil/actor/io/accept_handle.hpp>
#include <nil/actor/io/broker_servant.hpp>
#include <nil/actor/io/system_messages.hpp>
#include <nil/actor/io/network/acceptor_manager.hpp>

namespace nil {
    namespace actor {
        namespace io {

            using doorman_base = broker_servant<network::acceptor_manager, accept_handle, new_connection_msg>;

            /// Manages incoming connections.
            /// @ingroup Broker
            class doorman : public doorman_base {
            public:
                doorman(accept_handle acc_hdl);

                ~doorman() override;

                using doorman_base::new_connection;

                bool new_connection(execution_unit *ctx, connection_handle x);

                /// Starts listening on the selected port.
                virtual void launch() = 0;

            protected:
                message detach_message() override;
            };

            using doorman_ptr = intrusive_ptr<doorman>;

        }    // namespace io
    }        // namespace actor
}    // namespace nil

// Allows the `middleman_actor` to create a `doorman` and then send it to the
// BASP broker.
ACTOR_ALLOW_UNSAFE_MESSAGE_TYPE(nil::actor::io::doorman_ptr)
