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

#include <vector>

#include <nil/actor/message.hpp>

#include <nil/actor/io/broker_servant.hpp>
#include <nil/actor/io/receive_policy.hpp>
#include <nil/actor/io/system_messages.hpp>
#include <nil/actor/io/network/stream_manager.hpp>

namespace nil {
    namespace actor {
        namespace io {

            using scribe_base = broker_servant<network::stream_manager, connection_handle, new_data_msg>;

            /// Manages a stream.
            /// @ingroup Broker
            class scribe : public scribe_base {
            public:
                typedef byte_buffer buffer_type;

                scribe(connection_handle conn_hdl);

                ~scribe() override;

                /// Implicitly starts the read loop on first call.
                virtual void configure_read(receive_policy::config config) = 0;

                /// Enables or disables write notifications.
                virtual void ack_writes(bool enable) = 0;

                /// Returns the current output buffer.
                virtual byte_buffer &wr_buf() = 0;

                /// Returns the current input buffer.
                virtual byte_buffer &rd_buf() = 0;

                /// Flushes the output buffer, i.e., sends the
                /// content of the buffer via the network.
                virtual void flush() = 0;

                bool consume(execution_unit *, const void *, size_t) override;

                void data_transferred(execution_unit *, size_t, size_t) override;

            protected:
                message detach_message() override;
            };

            using scribe_ptr = intrusive_ptr<scribe>;

        }    // namespace io
    }        // namespace actor
}    // namespace nil

// Allows the `middleman_actor` to create a `scribe` and then send it to the
// BASP broker.
ACTOR_ALLOW_UNSAFE_MESSAGE_TYPE(nil::actor::io::scribe_ptr)
