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

#include <nil/mtl/io/fwd.hpp>
#include <nil/mtl/io/scribe.hpp>

#include <nil/mtl/io/network/stream_impl.hpp>
#include <nil/mtl/io/network/native_socket.hpp>

#include <nil/mtl/policy/tcp.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                /// Default scribe implementation.
                class scribe_impl : public scribe {
                public:
                    scribe_impl(default_multiplexer &mx, native_socket sockfd);

                    void configure_read(receive_policy::config config) override;

                    void ack_writes(bool enable) override;

                    byte_buffer &wr_buf() override;

                    byte_buffer &rd_buf() override;

                    void graceful_shutdown() override;

                    void flush() override;

                    std::string addr() const override;

                    uint16_t port() const override;

                    void launch();

                    void add_to_loop() override;

                    void remove_from_loop() override;

                protected:
                    bool launched_;
                    stream_impl<policy::tcp> stream_;
                };

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
