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

#include <nil/actor/core/future.hh>
#include <nil/actor/core/reactor.hh>

#include <nil/actor/cluster/detail/server.hpp>
#include <nil/actor/cluster/detail/membership.hpp>

namespace nil::actor {
    namespace cluster {
        nil::actor::future<> with_cluster_impl(nil::actor::socket_address const &local,
                                               std::vector<nil::actor::socket_address> &&peers);

        template<typename Func>
        nil::actor::future<> with_cluster(nil::actor::socket_address const &local,
                                          std::vector<nil::actor::socket_address> &&peers,
                                          std::size_t minimum_connected_peers, Func &&func) {
            return with_cluster_impl(local, std::move(peers))
                .then([func = std::forward<Func>(func), minimum_connected_peers]() mutable {
                    return detail::membership::service.local()
                        .joined_cv
                        .wait([minimum_connected_peers] {
                            return detail::membership::service.local().members().size() >= minimum_connected_peers;
                        })
                        .then([func = std::forward<Func>(func)] {
                            nil::actor::print("%d: Calling user code\n", nil::actor::engine().cpu_id());
                            return func();
                        });
                });
        }

        template<typename Func>
        nil::actor::future<> with_cluster(nil::actor::socket_address const &local,
                                          std::vector<nil::actor::socket_address> &&peers, Func &&func) {
            return with_cluster(local, std::move(peers), 0, std::forward<Func>(func));
        }

        template<typename Func>
        nil::actor::future<> with_cluster(nil::actor::socket_address const &local, std::size_t minimum_connected_peers,
                                          Func &&func) {
            return with_cluster(local, {}, minimum_connected_peers, std::forward<Func>(func));
        }

        template<typename Func>
        nil::actor::future<> with_cluster(nil::actor::socket_address const &local, Func &&func) {
            return with_cluster(local, {}, 0, std::forward<Func>(func));
        }
    }    // namespace cluster
}    // namespace nil::actor
