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
#include <nil/mtl/io/network/native_socket.hpp>
#include <nil/mtl/io/network/datagram_handler.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                /// A concrete datagram_handler with a technology-dependent policy.
                template<class ProtocolPolicy>
                class datagram_handler_impl : public datagram_handler {
                public:
                    template<class... Ts>
                    datagram_handler_impl(default_multiplexer &mpx, native_socket sockfd, Ts &&... xs) :
                        datagram_handler(mpx, sockfd), policy_(std::forward<Ts>(xs)...) {
                        // nop
                    }

                    void handle_event(io::network::operation op) override {
                        this->handle_event_impl(op, policy_);
                    }

                private:
                    ProtocolPolicy policy_;
                };

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
