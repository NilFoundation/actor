//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2018-2020 Nil Foundation AG
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE datagram_socket

#include <nil/actor/network/datagram_socket.hpp>

#include <nil/actor/test/dsl.hpp>

using namespace nil::actor;
using namespace nil::actor::network;

BOOST_AUTO_TEST_CASE(invalid_socket) {
    datagram_socket x;
    BOOST_CHECK_NE(allow_connreset(x, true), none);
}
