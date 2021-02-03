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

#include <nil/actor/cluster/detail/message_serializer.hpp>

namespace nil::actor {
    namespace cluster {
        namespace impl {
            struct handshake_request {
                std::vector<nil::actor::socket_address> known_nodes;
                nil::actor::socket_address origin;

                handshake_request(std::vector<nil::actor::socket_address> peers, nil::actor::socket_address const &origin);

                template<typename Serializer, typename Output>
                inline void serialize(Serializer s, Output &out) const {
                    write(s, out, known_nodes);
                    write(s, out, origin);
                }

                template<typename Serializer, typename Input>
                static inline handshake_request deserialize(Serializer s, Input &in) {
                    auto known_nodes = read(s, in, nil::actor::rpc::type<std::vector<nil::actor::socket_address>> {});
                    auto origin = read(s, in, nil::actor::rpc::type<nil::actor::socket_address> {});
                    return handshake_request(std::move(known_nodes), origin);
                }
            };

            struct handshake_response {
                std::vector<nil::actor::socket_address> known_nodes;
                std::size_t shard_count;

                explicit handshake_response(std::vector<nil::actor::socket_address> peers,
                                            std::size_t shard_count = nil::actor::smp::count);

                template<typename Serializer, typename Output>
                inline void serialize(Serializer s, Output &out) const {
                    write(s, out, known_nodes);
                    write(s, out, shard_count);
                }

                template<typename Serializer, typename Input>
                static inline handshake_response deserialize(Serializer s, Input &in) {
                    auto known_nodes = read(s, in, nil::actor::rpc::type<std::vector<nil::actor::socket_address>> {});
                    auto shard_count = read(s, in, nil::actor::rpc::type<std::size_t> {});
                    return handshake_response(std::move(known_nodes), shard_count);
                }
            };
        }    // namespace impl
    }        // namespace cluster
}    // namespace nil::actor
