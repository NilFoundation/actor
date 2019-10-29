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

#include <cstddef>
#include <cstdint>

#include <nil/mtl/message.hpp>
#include <nil/mtl/mailbox_element.hpp>

#include <nil/mtl/io/accept_handle.hpp>
#include <nil/mtl/io/broker_servant.hpp>
#include <nil/mtl/io/system_messages.hpp>
#include <nil/mtl/io/network/acceptor_manager.hpp>

namespace nil {
    namespace mtl {
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
    }        // namespace mtl
}    // namespace nil

// Allows the `middleman_actor` to create a `doorman` and then send it to the
// BASP broker.
MTL_ALLOW_UNSAFE_MESSAGE_TYPE(nil::mtl::io::doorman_ptr)
