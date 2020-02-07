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

#include <nil/mtl/io/network/event_handler.hpp>

#include <nil/mtl/logger.hpp>

#include <nil/mtl/io/network/default_multiplexer.hpp>

#ifdef MTL_WINDOWS
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                event_handler::event_handler(default_multiplexer &dm, native_socket sockfd) :
                    fd_(sockfd), state_ {true, false, false, false, receive_policy_flag::at_least}, eventbf_(0),
                    backend_(dm) {
                    set_fd_flags();
                }

                event_handler::~event_handler() {
                    if (fd_ != invalid_native_socket) {
                        MTL_LOG_DEBUG("close socket" << MTL_ARG(fd_));
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
    }            // namespace mtl
}    // namespace nil
