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

#include <nil/actor/cluster/cluster.hpp>

#include <nil/actor/core/future-util.hh>
#include <nil/actor/core/sleep.hh>

namespace nil {
    namespace actor {
        namespace cluster {
            static nil::actor::future<> try_join(nil::actor::socket_address const &local,
                                                 std::vector<nil::actor::socket_address> const &peers) {
                return detail::server::service.start(local).then([&local, &peers] {
                    return detail::membership::service.start(local).then([&peers] {
                        return nil::actor::parallel_for_each(
                                   peers,
                                   [](nil::actor::socket_address const &peer) {
                                       return impl::membership::service.invoke_on_all(
                                           [peer](auto &service) { return service.try_add_peer(peer); });
                                   })
                            .then([&peers] {
                                if (!peers.empty() && !detail::membership::service.local().is_connected_to_cluster()) {
                                    return nil::actor::when_all(detail::membership::service.stop(),
                                                                detail::server::service.stop())
                                        .then([](auto) {
                                            return nil::actor::make_exception_future(
                                                std::runtime_error("Failed to join cluster"));
                                        });

                                } else if (peers.empty()) {
                                    nil::actor::print("%d: No cluster to join, assuming bootstrap node\n",
                                                      nil::actor::engine().cpu_id());
                                }
                                return nil::actor::make_ready_future();
                            });
                    });
                });
            };

            nil::actor::future<> with_cluster_impl(nil::actor::socket_address const &local,
                                                   std::vector<nil::actor::socket_address> &&peers) {
                return do_with(
                    int {0}, local, std::move(peers),
                    [](int &i, nil::actor::socket_address const &local,
                       std::vector<nil::actor::socket_address> const &peers) {
                        return nil::actor::repeat([&i, &local, &peers] {
                                   return try_join(local, peers)
                                       .then([] { return nil::actor::stop_iteration::yes; })
                                       .then_wrapped([&i](auto &&fut) {
                                           if (fut.failed()) {
                                               try {
                                                   std::rethrow_exception(fut.get_exception());
                                               } catch (std::system_error const &) {
                                                   throw;
                                               } catch (std::runtime_error const &ex) {
                                                   ++i;
                                                   nil::actor::print("%d: Failed to join cluster (%d/5 try)\n",
                                                                     nil::actor::engine().cpu_id(), i);
                                                   if (i >= 5) {
                                                       throw ex;
                                                   }
                                                   int backoff = std::pow<int>(2, i);
                                                   nil::actor::print("%d: Retrying in %d seconds...\n",
                                                                     nil::actor::engine().cpu_id(), backoff);
                                                   return nil::actor::sleep(std::chrono::seconds(backoff)).then([] {
                                                       return nil::actor::stop_iteration::no;
                                                   });
                                               }
                                           }
                                           return nil::actor::make_ready_future<
                                               nil::actor::bool_class<nil::actor::stop_iteration_tag>>(
                                               nil::actor::stop_iteration::yes);
                                       });
                               })
                            .then([] {
                                nil::actor::engine().at_exit([] {
                                    return nil::actor::when_all(detail::membership::service.stop(),
                                                                detail::server::service.stop())
                                        .discard_result();
                                });
                            })
                            .handle_exception(
                                [](std::exception_ptr ex) { return nil::actor::make_exception_future(ex); });
                    });
            }
        }    // namespace cluster
    }        // namespace actor
}    // namespace nil
