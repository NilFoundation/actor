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

#include <nil/actor/io/fwd.hpp>

#include <nil/actor/io/network/acceptor.hpp>
#include <nil/actor/io/network/operation.hpp>
#include <nil/actor/io/network/native_socket.hpp>

namespace nil {
    namespace actor {
        namespace io {
            namespace network {

                /// A concrete acceptor with a technology-dependent policy.
                template<class ProtocolPolicy>
                class acceptor_impl : public acceptor {
                public:
                    template<class... Ts>
                    acceptor_impl(default_multiplexer &mpx, native_socket sockfd, Ts &&... xs) :
                        acceptor(mpx, sockfd), policy_(std::forward<Ts>(xs)...) {
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
    }            // namespace actor
}    // namespace nil
