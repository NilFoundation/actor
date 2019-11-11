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

#include <nil/mtl/io/network/datagram_servant_impl.hpp>

#include <algorithm>

#include <nil/mtl/logger.hpp>

#include <nil/mtl/io/network/default_multiplexer.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                datagram_servant_impl::datagram_servant_impl(default_multiplexer &mx, native_socket sockfd,
                                                             int64_t id) :
                    datagram_servant(datagram_handle::from_int(id)),
                    launched_(false), handler_(mx, sockfd) {
                    // nop
                }

                bool datagram_servant_impl::new_endpoint(network::receive_buffer &buf) {
                    MTL_LOG_TRACE("");
                    if (detached())
                        // We are already disconnected from the broker while the multiplexer
                        // did not yet remove the socket, this can happen if an I/O event
                        // causes the broker to call close_all() while the pollset contained
                        // further activities for the broker.
                        return false;
                    // A datagram that has a source port of zero is valid and never requires a
                    // reply. In the case of MTL we can simply drop it as nothing but the
                    // handshake could be communicated which we could not reply to.
                    // Source: TCP/IP Illustrated, Chapter 10.2
                    if (network::port(handler_.sending_endpoint()) == 0)
                        return true;
                    auto &dm = handler_.backend();
                    auto hdl = datagram_handle::from_int(dm.next_endpoint_id());
                    add_endpoint(handler_.sending_endpoint(), hdl);
                    parent()->add_hdl_for_datagram_servant(this, hdl);
                    return consume(&dm, hdl, buf);
                }

                void datagram_servant_impl::ack_writes(bool enable) {
                    MTL_LOG_TRACE(MTL_ARG(enable));
                    handler_.ack_writes(enable);
                }

                std::vector<char> &datagram_servant_impl::wr_buf(datagram_handle hdl) {
                    return handler_.wr_buf(hdl);
                }

                void datagram_servant_impl::enqueue_datagram(datagram_handle hdl, std::vector<char> buffer) {
                    handler_.enqueue_datagram(hdl, std::move(buffer));
                }

                network::receive_buffer &datagram_servant_impl::rd_buf() {
                    return handler_.rd_buf();
                }

                void datagram_servant_impl::graceful_shutdown() {
                    MTL_LOG_TRACE("");
                    handler_.graceful_shutdown();
                    detach_handles();
                    detach(&handler_.backend(), false);
                }

                void datagram_servant_impl::flush() {
                    MTL_LOG_TRACE("");
                    handler_.flush(this);
                }

                std::string datagram_servant_impl::addr() const {
                    auto x = remote_addr_of_fd(handler_.fd());
                    if (!x)
                        return "";
                    return *x;
                }

                uint16_t datagram_servant_impl::port(datagram_handle hdl) const {
                    auto &eps = handler_.endpoints();
                    auto itr = eps.find(hdl);
                    if (itr == eps.end())
                        return 0;
                    return network::port(itr->second);
                }

                uint16_t datagram_servant_impl::local_port() const {
                    auto x = local_port_of_fd(handler_.fd());
                    if (!x)
                        return 0;
                    return *x;
                }

                std::vector<datagram_handle> datagram_servant_impl::hdls() const {
                    std::vector<datagram_handle> result;
                    result.reserve(handler_.endpoints().size());
                    for (auto &p : handler_.endpoints())
                        result.push_back(p.first);
                    return result;
                }

                void datagram_servant_impl::add_endpoint(const ip_endpoint &ep, datagram_handle hdl) {
                    handler_.add_endpoint(hdl, ep, this);
                }

                void datagram_servant_impl::remove_endpoint(datagram_handle hdl) {
                    handler_.remove_endpoint(hdl);
                }

                void datagram_servant_impl::launch() {
                    MTL_LOG_TRACE("");
                    MTL_ASSERT(!launched_);
                    launched_ = true;
                    handler_.start(this);
                }

                void datagram_servant_impl::add_to_loop() {
                    handler_.activate(this);
                }

                void datagram_servant_impl::remove_from_loop() {
                    handler_.passivate();
                }

                void datagram_servant_impl::detach_handles() {
                    for (auto &p : handler_.endpoints()) {
                        if (p.first != hdl())
                            parent()->erase(p.first);
                    }
                }

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
