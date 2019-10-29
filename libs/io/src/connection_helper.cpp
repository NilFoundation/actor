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

#include <nil/mtl/io/connection_helper.hpp>

#include <chrono>

#include <nil/mtl/defaults.hpp>
#include <nil/mtl/io/basp/instance.hpp>

namespace nil {
    namespace mtl {
        namespace io {

            namespace {

                auto autoconnect_timeout = std::chrono::minutes(10);

            }    // namespace

            const char *connection_helper_state::name = "connection_helper";

            behavior connection_helper(stateful_actor<connection_helper_state> *self, actor b) {
                MTL_LOG_TRACE(MTL_ARG(b));
                self->monitor(b);
                self->set_down_handler([=](down_msg &dm) {
                    MTL_LOG_TRACE(MTL_ARG(dm));
                    self->quit(std::move(dm.reason));
                });
                return {// this config is send from the remote `ConfigServ`
                        [=](const std::string &item, message &msg) {
                            MTL_LOG_TRACE(MTL_ARG(item) << MTL_ARG(msg));
                            MTL_LOG_DEBUG("received requested config:" << MTL_ARG(msg));
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
                                                MTL_LOG_INFO("connected directly:" << MTL_ARG(addr));
                                                self->send(b, connect_atom::value, *hdl, port);
                                                return;
                                            }
                                        }
                                    }
                                    MTL_LOG_INFO("could not connect to node directly");
                                } else {
                                    MTL_LOG_INFO("aborted direct connection attempt, unknown item: " << MTL_ARG(item));
                                }
                            }});
                        },
                        after(autoconnect_timeout) >>
                            [=] {
                                MTL_LOG_TRACE(MTL_ARG(""));
                                // nothing heard in about 10 minutes... just a call it a day, then
                                MTL_LOG_INFO("aborted direct connection attempt after 10min");
                                self->quit(exit_reason::user_shutdown);
                            }};
            }

        }    // namespace io
    }        // namespace mtl
}    // namespace nil
