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

#include <nil/mtl/io/network/scribe_impl.hpp>

#include <algorithm>

#include <nil/mtl/logger.hpp>

#include <nil/mtl/io/network/default_multiplexer.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                scribe_impl::scribe_impl(default_multiplexer &mx, native_socket sockfd) :
                    scribe(network::conn_hdl_from_socket(sockfd)), launched_(false), stream_(mx, sockfd) {
                    // nop
                }

                void scribe_impl::configure_read(receive_policy::config config) {
                    MTL_LOG_TRACE("");
                    stream_.configure_read(config);
                    if (!launched_)
                        launch();
                }

                void scribe_impl::ack_writes(bool enable) {
                    MTL_LOG_TRACE(MTL_ARG(enable));
                    stream_.ack_writes(enable);
                }

                std::vector<char> &scribe_impl::wr_buf() {
                    return stream_.wr_buf();
                }

                std::vector<char> &scribe_impl::rd_buf() {
                    return stream_.rd_buf();
                }

                void scribe_impl::graceful_shutdown() {
                    MTL_LOG_TRACE("");
                    stream_.graceful_shutdown();
                    detach(&stream_.backend(), false);
                }

                void scribe_impl::flush() {
                    MTL_LOG_TRACE("");
                    stream_.flush(this);
                }

                std::string scribe_impl::addr() const {
                    auto x = remote_addr_of_fd(stream_.fd());
                    if (!x)
                        return "";
                    return *x;
                }

                uint16_t scribe_impl::port() const {
                    auto x = remote_port_of_fd(stream_.fd());
                    if (!x)
                        return 0;
                    return *x;
                }

                void scribe_impl::launch() {
                    MTL_LOG_TRACE("");
                    MTL_ASSERT(!launched_);
                    launched_ = true;
                    stream_.start(this);
                }

                void scribe_impl::add_to_loop() {
                    stream_.activate(this);
                }

                void scribe_impl::remove_from_loop() {
                    stream_.passivate();
                }

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
