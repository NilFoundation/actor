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

#include <nil/actor/cluster/detail/message_serializer.hpp>
#include <nil/actor/cluster/detail/node.hpp>
#include <nil/actor/cluster/detail/message_handler_registry.hpp>

namespace ultramarine {
    namespace cluster {
        namespace detail {
            class server {
            private:
                rpc_proto proto {serializer {}};
                std::unique_ptr<rpc_proto::server> rpc;
                nil::actor::socket_address local;

            public:
                explicit server(nil::actor::socket_address const &local);

                nil::actor::future<> stop();
                static inline nil::actor::sharded<server> service;
                static inline nil::actor::future<> start(nil::actor::socket_address const &local);
            };
        }    // namespace detail
    }        // namespace cluster
}    // namespace ultramarine
