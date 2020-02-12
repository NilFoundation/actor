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

#pragma once

#include <nil/actor/io/network/native_socket.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            class socket_guard {
            public:
                explicit socket_guard(io::network::native_socket fd);

                ~socket_guard();

                io::network::native_socket release();

                void close();

            private:
                io::network::native_socket fd_;
            };

        }    // namespace detail
    }        // namespace actor
}    // namespace nil
