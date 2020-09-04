//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/io/network/event_handler.hpp>

#include <nil/actor/logger.hpp>

#include <nil/actor/io/network/default_multiplexer.hpp>

#ifdef BOOST_OS_WINDOWS_AVAILABLE
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

namespace nil {
    namespace actor {
        namespace io {
            namespace network {

                event_handler::event_handler(default_multiplexer &dm, native_socket sockfd) :
                    fd_(sockfd), state_ {true, false, false, false, receive_policy_flag::at_least}, eventbf_(0),
                    backend_(dm) {
                    set_fd_flags();
                }

                event_handler::~event_handler() {
                    if (fd_ != invalid_native_socket) {
                        ACTOR_LOG_DEBUG("close socket" << ACTOR_ARG(fd_));
                        close_socket(fd_);
                    }
                }

                void event_handler::passivate() {
                    backend().del(operation::read, fd(), this);
                }

                void event_handler::activate() {
                    backend().add(operation::read, fd(), this);
                }

                void event_handler::set_fd_flags() {
                    if (fd_ == invalid_native_socket)
                        return;
                    // enable nonblocking IO, disable Nagle's algorithm, and suppress SIGPIPE
                    nonblocking(fd_, true);
                    tcp_nodelay(fd_, true);
                    allow_sigpipe(fd_, false);
                }

            }    // namespace network
        }        // namespace io
    }            // namespace actor
}    // namespace nil
