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
#include <nil/actor/io/doorman.hpp>

#include <nil/actor/io/network/acceptor_impl.hpp>
#include <nil/actor/io/network/native_socket.hpp>

#include <nil/actor/policy/tcp.hpp>

namespace nil {
    namespace actor {
        namespace io {
            namespace network {

                /// Default doorman implementation.
                class doorman_impl : public doorman {
                public:
                    doorman_impl(default_multiplexer &mx, native_socket sockfd);

                    bool new_connection() override;

                    void graceful_shutdown() override;

                    void launch() override;

                    std::string addr() const override;

                    uint16_t port() const override;

                    void add_to_loop() override;

                    void remove_from_loop() override;

                protected:
                    acceptor_impl<policy::tcp> acceptor_;
                };

            }    // namespace network
        }        // namespace io
    }            // namespace actor
}    // namespace nil
