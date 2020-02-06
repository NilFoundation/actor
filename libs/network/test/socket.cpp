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

#define BOOST_TEST_MODULE socket

#include <nil/mtl/network/socket.hpp>

#include <nil/mtl/test/host_fixture.hpp>
#include <nil/mtl/test/dsl.hpp>

using namespace nil::mtl;
using namespace nil::mtl::network;

BOOST_FIXTURE_TEST_SUITE(socket_tests, host_fixture)

BOOST_AUTO_TEST_CASE(invalid socket) {
    auto x = invalid_socket;
    BOOST_CHECK_EQUAL(x.id, invalid_socket_id);
    BOOST_CHECK_EQUAL(child_process_inherit(x, true), sec::network_syscall_failed);
    BOOST_CHECK_EQUAL(nonblocking(x, true), sec::network_syscall_failed);
}

BOOST_AUTO_TEST_SUITE_END()
