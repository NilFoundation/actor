//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/detail/socket_guard.hpp>

#ifdef ACTOR_WINDOWS
#include <winsock2.h>
#else
#include <unistd.h>
#endif

#include <nil/actor/logger.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            socket_guard::socket_guard(io::network::native_socket fd) : fd_(fd) {
                // nop
            }

            socket_guard::~socket_guard() {
                close();
            }

            io::network::native_socket socket_guard::release() {
                auto fd = fd_;
                fd_ = io::network::invalid_native_socket;
                return fd;
            }

            void socket_guard::close() {
                if (fd_ != io::network::invalid_native_socket) {
                    ACTOR_LOG_DEBUG("close socket" << ACTOR_ARG(fd_));
                    io::network::close_socket(fd_);
                    fd_ = io::network::invalid_native_socket;
                }
            }

        }    // namespace detail
    }        // namespace actor
}    // namespace nil