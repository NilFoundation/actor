//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2018-2020 Nil Foundation AG
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <vector>

#include <nil/mtl/logger.hpp>
#include <nil/mtl/network/make_endpoint_manager.hpp>
#include <nil/mtl/network/socket.hpp>
#include <nil/mtl/network/stream_socket.hpp>
#include <nil/mtl/network/stream_transport.hpp>
#include <nil/mtl/network/tcp_accept_socket.hpp>
#include <nil/mtl/network/tcp_stream_socket.hpp>
#include <nil/mtl/send.hpp>

namespace nil {
    namespace mtl {
        namespace network {

            /// A doorman accepts TCP connections and creates stream_transports to handle
            /// them.
            template<class Factory>
            class doorman {
            public:
                // -- member types -----------------------------------------------------------

                using factory_type = Factory;

                using application_type = typename Factory::application_type;

                // -- constructors, destructors, and assignment operators --------------------

                explicit doorman(net::tcp_accept_socket acceptor, factory_type factory) :
                    acceptor_(acceptor), factory_(std::move(factory)) {
                    // nop
                }

                // -- properties -------------------------------------------------------------

                net::tcp_accept_socket handle() {
                    return acceptor_;
                }

                // -- member functions -------------------------------------------------------

                template<class Parent>
                error init(Parent &parent) {
                    // TODO: is initializing application factory nessecary?
                    if (auto err = factory_.init(parent))
                        return err;
                    return none;
                }

                template<class Parent>
                bool handle_read_event(Parent &parent) {
                    auto x = net::accept(acceptor_);
                    if (!x) {
                        MTL_LOG_ERROR("accept failed:" << parent.system().render(x.error()));
                        return false;
                    }
                    auto mpx = parent.multiplexer();
                    if (!mpx) {
                        MTL_LOG_DEBUG("unable to get multiplexer from parent");
                        return false;
                    }
                    auto child = make_endpoint_manager(mpx, parent.system(),
                                                       stream_transport<application_type> {*x, factory_.make()});
                    if (auto err = child->init())
                        return false;
                    return true;
                }

                template<class Parent>
                bool handle_write_event(Parent &) {
                    MTL_LOG_ERROR("doorman received write event");
                    return false;
                }

                template<class Parent>
                void resolve(Parent &, const uri &locator, const actor &listener) {
                    MTL_LOG_ERROR("doorman called to resolve" << MTL_ARG(locator));
                    anon_send(listener, resolve_atom_v, "doormen cannot resolve paths");
                }

                void new_proxy(endpoint_manager &, const node_id &peer, actor_id id) {
                    MTL_LOG_ERROR("doorman received new_proxy" << MTL_ARG(peer) << MTL_ARG(id));
                    MTL_IGNORE_UNUSED(peer);
                    MTL_IGNORE_UNUSED(id);
                }

                void local_actor_down(endpoint_manager &, const node_id &peer, actor_id id, error reason) {
                    MTL_LOG_ERROR("doorman received local_actor_down" << MTL_ARG(peer) << MTL_ARG(id)
                                                                      << MTL_ARG(reason));
                    MTL_IGNORE_UNUSED(peer);
                    MTL_IGNORE_UNUSED(id);
                    MTL_IGNORE_UNUSED(reason);
                }

                template<class Parent>
                void timeout(Parent &, [[maybe_unused]] const std::string &tag, [[maybe_unused]] uint64_t id) {
                    MTL_LOG_ERROR("doorman received timeout" << MTL_ARG(tag) << MTL_ARG(id));
                }

                void handle_error(sec err) {
                    MTL_LOG_ERROR("doorman encounterd error: " << err);
                }

            private:
                net::tcp_accept_socket acceptor_;

                factory_type factory_;
            };
        }    // namespace network
    }        // namespace mtl
}    // namespace nil