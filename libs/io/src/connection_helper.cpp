//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/io/connection_helper.hpp>

#include <chrono>

#include <nil/actor/defaults.hpp>
#include <nil/actor/io/basp/instance.hpp>

namespace nil {
    namespace actor {
        namespace io {

            namespace {

                auto autoconnect_timeout = std::chrono::minutes(10);

            }    // namespace

            const char *connection_helper_state::name = "connection_helper";

            behavior connection_helper(stateful_actor<connection_helper_state> *self, actor b) {
                ACTOR_LOG_TRACE(ACTOR_ARG(b));
                self->monitor(b);
                self->set_down_handler([=](down_msg &dm) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(dm));
                    self->quit(std::move(dm.reason));
                });
                return {// this config is send from the remote `ConfigServ`
                        [=](const std::string &item, message &msg) {
                            ACTOR_LOG_TRACE(ACTOR_ARG(item) << ACTOR_ARG(msg));
                            ACTOR_LOG_DEBUG("received requested config:" << ACTOR_ARG(msg));
                            // whatever happens, we are done afterwards
                            self->quit();
                            msg.apply({[&](uint16_t port, network::address_listing &addresses) {
                                if (item == "basp.default-connectivity-tcp") {
                                    auto &mx = self->system().middleman().backend();
                                    for (auto &kvp : addresses) {
                                        for (auto &addr : kvp.second) {
                                            auto hdl = mx.new_tcp_scribe(addr, port);
                                            if (hdl) {
                                                // gotcha! send scribe to our BASP broker
                                                // to initiate handshake etc.
                                                ACTOR_LOG_INFO("connected directly:" << ACTOR_ARG(addr));
                                                self->send(b, connect_atom::value, *hdl, port);
                                                return;
                                            }
                                        }
                                    }
                                    ACTOR_LOG_INFO("could not connect to node directly");
                                } else {
                                    ACTOR_LOG_INFO("aborted direct connection attempt, unknown item: " << ACTOR_ARG(item));
                                }
                            }});
                        },
                        after(autoconnect_timeout) >>
                            [=] {
                                ACTOR_LOG_TRACE(ACTOR_ARG(""));
                                // nothing heard in about 10 minutes... just a call it a day, then
                                ACTOR_LOG_INFO("aborted direct connection attempt after 10min");
                                self->quit(exit_reason::user_shutdown);
                            }};
            }

        }    // namespace io
    }        // namespace actor
}    // namespace nil
