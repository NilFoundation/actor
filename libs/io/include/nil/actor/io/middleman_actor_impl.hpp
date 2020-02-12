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

#include <nil/actor/fwd.hpp>
#include <nil/actor/atom.hpp>
#include <nil/actor/typed_actor.hpp>
#include <nil/actor/typed_event_based_actor.hpp>

#include <nil/actor/io/fwd.hpp>
#include <nil/actor/io/middleman_actor.hpp>

namespace nil {
    namespace actor {
        namespace io {

            /// Default implementation of the `middleman_actor` interface.
            class middleman_actor_impl : public middleman_actor::base {
            public:
                using put_res = result<uint16_t>;

                using mpi_set = std::set<std::string>;

                using get_res = result<node_id, strong_actor_ptr, mpi_set>;

                using get_delegated = delegated<node_id, strong_actor_ptr, mpi_set>;

                using del_res = result<void>;

                using endpoint_data = std::tuple<node_id, strong_actor_ptr, mpi_set>;

                using endpoint = std::pair<std::string, uint16_t>;

                middleman_actor_impl(actor_config &cfg, actor default_broker);

                void on_exit() override;

                const char *name() const override;

                behavior_type make_behavior() override;

            protected:
                /// Tries to connect to given `host` and `port`. The default implementation
                /// calls `system().middleman().backend().new_tcp_scribe(host, port)`.
                virtual expected<scribe_ptr> connect(const std::string &host, uint16_t port);

                /// Tries to connect to given `host` and `port`. The default implementation
                /// calls `system().middleman().backend().new_udp`.
                virtual expected<datagram_servant_ptr> contact(const std::string &host, uint16_t port);

                /// Tries to open a local port. The default implementation calls
                /// `system().middleman().backend().new_tcp_doorman(port, addr, reuse)`.
                virtual expected<doorman_ptr> open(uint16_t port, const char *addr, bool reuse);

                /// Tries to open a local port. The default implementation calls
                /// `system().middleman().backend().new_tcp_doorman(port, addr, reuse)`.
                virtual expected<datagram_servant_ptr> open_udp(uint16_t port, const char *addr, bool reuse);

            private:
                put_res put(uint16_t port, strong_actor_ptr &whom, mpi_set &sigs, const char *in = nullptr,
                            bool reuse_addr = false);

                put_res put_udp(uint16_t port, strong_actor_ptr &whom, mpi_set &sigs, const char *in = nullptr,
                                bool reuse_addr = false);

                optional<endpoint_data &> cached_tcp(const endpoint &ep);
                optional<endpoint_data &> cached_udp(const endpoint &ep);

                optional<std::vector<response_promise> &> pending(const endpoint &ep);

                actor broker_;
                std::map<endpoint, endpoint_data> cached_tcp_;
                std::map<endpoint, endpoint_data> cached_udp_;
                std::map<endpoint, std::vector<response_promise>> pending_;
            };

        }    // namespace io
    }        // namespace actor
}    // namespace nil
