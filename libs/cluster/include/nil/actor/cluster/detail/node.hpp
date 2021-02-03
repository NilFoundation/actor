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

#include <nil/actor/net/inet_address.hh>
#include <nil/actor/net/ip.hh>

#include <nil/actor/cluster/detail/message_serializer.hpp>

namespace nil::actor {
    namespace cluster {
        namespace detail {
            struct node {
                nil::actor::socket_address endpoint;
                nil::actor::lw_shared_ptr<rpc_proto::client> client;
                rpc_proto *rpc;

                node(uint32_t ip4, uint16_t port);
                node(rpc_proto *proto, nil::actor::lw_shared_ptr<rpc_proto::client> &&client);

                node(node const &) = default;

                node(node &&) = default;

                node &operator=(node const &n) = default;

                node &operator=(node &&n) = default;

                bool operator==(const node &rhs) const;

                bool operator!=(const node &rhs) const;

                explicit operator nil::actor::socket_address() const;
            };
        }    // namespace detail
    }        // namespace cluster
}    // namespace nil::actor

namespace std {
    template<>
    struct hash<nil::actor::cluster::detail::node> {
        size_t operator()(nil::actor::cluster::detail::node const &node) const;
    };
}    // namespace std

