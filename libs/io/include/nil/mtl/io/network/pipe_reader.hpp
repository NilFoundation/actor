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

#include <nil/mtl/io/network/operation.hpp>
#include <nil/mtl/io/network/event_handler.hpp>
#include <nil/mtl/io/network/native_socket.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                /// An event handler for the internal event pipe.
                class pipe_reader : public event_handler {
                public:
                    pipe_reader(default_multiplexer &dm);

                    void removed_from_loop(operation op) override;

                    void graceful_shutdown() override;

                    void handle_event(operation op) override;

                    void init(native_socket sock_fd);

                    resumable *try_read_next();
                };

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
