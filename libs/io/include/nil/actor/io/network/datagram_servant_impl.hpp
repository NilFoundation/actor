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

#include <nil/actor/io/fwd.hpp>
#include <nil/actor/io/datagram_servant.hpp>

#include <nil/actor/io/network/native_socket.hpp>
#include <nil/actor/io/network/datagram_handler_impl.hpp>

#include <nil/actor/policy/udp.hpp>

namespace nil {
    namespace actor {
        namespace io {
            namespace network {

                /// Default datagram servant implementation.
                class datagram_servant_impl : public datagram_servant {
                    using id_type = int64_t;

                public:
                    datagram_servant_impl(default_multiplexer &mx, native_socket sockfd, int64_t id);

                    bool new_endpoint(network::receive_buffer &buf) override;

                    void ack_writes(bool enable) override;

                    byte_buffer &wr_buf(datagram_handle hdl) override;

                    void enqueue_datagram(datagram_handle hdl, byte_buffer buf) override;

                    network::receive_buffer &rd_buf() override;

                    void graceful_shutdown() override;

                    void flush() override;

                    std::string addr() const override;

                    uint16_t port(datagram_handle hdl) const override;

                    uint16_t local_port() const override;

                    std::vector<datagram_handle> hdls() const override;

                    void add_endpoint(const ip_endpoint &ep, datagram_handle hdl) override;

                    void remove_endpoint(datagram_handle hdl) override;

                    void launch() override;

                    void add_to_loop() override;

                    void remove_from_loop() override;

                    void detach_handles() override;

                private:
                    bool launched_;
                    datagram_handler_impl<policy::udp> handler_;
                };

            }    // namespace network
        }        // namespace io
    }            // namespace actor
}    // namespace nil
