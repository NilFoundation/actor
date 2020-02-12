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

#include <vector>

#include <nil/actor/message.hpp>

#include <nil/actor/io/datagram_handle.hpp>
#include <nil/actor/io/broker_servant.hpp>
#include <nil/actor/io/system_messages.hpp>
#include <nil/actor/io/network/ip_endpoint.hpp>
#include <nil/actor/io/network/datagram_manager.hpp>
#include <nil/actor/io/network/receive_buffer.hpp>

namespace nil {
    namespace actor {
        namespace io {

            using datagram_servant_base = broker_servant<network::datagram_manager, datagram_handle, new_datagram_msg>;

            /// Manages writing to a datagram sink.
            /// @ingroup Broker
            class datagram_servant : public datagram_servant_base {
            public:
                datagram_servant(datagram_handle hdl);

                ~datagram_servant() override;

                /// Enables or disables write notifications.
                virtual void ack_writes(bool enable) = 0;

                /// Returns a new output buffer.
                virtual byte_buffer &wr_buf(datagram_handle) = 0;

                /// Enqueue a buffer to be sent as a datagram.
                virtual void enqueue_datagram(datagram_handle, byte_buffer) = 0;

                /// Returns the current input buffer.
                virtual network::receive_buffer &rd_buf() = 0;

                /// Flushes the output buffer, i.e., sends the
                /// content of the buffer via the network.
                virtual void flush() = 0;

                /// Returns the local port of associated socket.
                virtual uint16_t local_port() const = 0;

                /// Returns all the handles associated with this servant
                virtual std::vector<datagram_handle> hdls() const = 0;

                /// Adds a new remote endpoint identified by the `ip_endpoint` to
                /// the related manager.
                virtual void add_endpoint(const network::ip_endpoint &ep, datagram_handle hdl) = 0;

                virtual void remove_endpoint(datagram_handle hdl) = 0;

                bool consume(execution_unit *, datagram_handle hdl, network::receive_buffer &buf) override;

                void datagram_sent(execution_unit *ctx, datagram_handle hdl, size_t written,
                                   byte_buffer buffer) override;

                virtual void detach_handles() = 0;

                using datagram_servant_base::new_endpoint;

                virtual void launch() = 0;

            protected:
                message detach_message() override;
            };

            using datagram_servant_ptr = intrusive_ptr<datagram_servant>;

        }    // namespace io
    }        // namespace actor
}    // namespace nil

// Allows the `middleman_actor` to create an `datagram_servant` and then send it
// to the BASP broker.
ACTOR_ALLOW_UNSAFE_MESSAGE_TYPE(nil::actor::io::datagram_servant_ptr)
