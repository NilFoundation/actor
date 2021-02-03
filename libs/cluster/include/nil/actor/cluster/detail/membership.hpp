//---------------------------------------------------------------------------//
// Copyright (c) 2018-2021 Mikhail Komarov <nemo@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#pragma once

#include <memory>

#include <nil/crypto3/hash/algorithm/hash.hpp>

#include <nil/actor/core/weak_ptr.hh>

#include <nil/actor/cluster/detail/node.hpp>
#include <nil/actor/cluster/detail/handshake.hpp>

typedef struct hash_ring_t hash_ring_t;

namespace nil::actor {
    namespace cluster {
        namespace impl {
            class membership : public nil::actor::weakly_referencable<membership> {
                using ring_ptr = std::unique_ptr<hash_ring_t, void (*)(hash_ring_t *)>;
                nil::actor::queue<nil::actor::socket_address> candidates;
                nil::actor::future<> candidate_connection_job;
                nil::actor::gate candidate_connection_gate;
                std::unordered_map<std::string, node> nodes;
                std::unordered_set<nil::actor::socket_address> connecting_nodes;
                ring_ptr ring;
                nil::actor::socket_address local_node;
                rpc_proto proto {serializer {}};

            public:
                nil::actor::condition_variable joined_cv;

                explicit membership(nil::actor::socket_address const &local);

                nil::actor::future<> try_add_peer(nil::actor::socket_address endpoint);

                node const *node_for_key(std::size_t key) const;

                bool is_connected_to_cluster() const;

                std::unordered_map<std::string, node> const &members() const;

                nil::actor::future<> stop();

                nil::actor::future<> add_candidates(handshake_request req);

                nil::actor::future<> contact_candidates();

                static inline nil::actor::sharded<membership> service;

                static inline nil::actor::future<> start(nil::actor::socket_address const &local);

            private:
                nil::actor::future<nil::actor::lw_shared_ptr<rpc_proto::client>>
                    connect(nil::actor::socket_address const &to);

                nil::actor::future<handshake_response> handshake(nil::actor::lw_shared_ptr<rpc_proto::client> &with);

                nil::actor::future<> disconnect(node const &n) const;
            };
        }    // namespace impl
    }        // namespace cluster
}    // namespace nil::actor
