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

#define BOOST_TEST_MODULE basp.header

#include <nil/mtl/network/basp/header.hpp>

#include <nil/mtl/test/dsl.hpp>

#include <nil/mtl/serialization/binary_deserializer.hpp>
#include <nil/mtl/serialization/binary_serializer.hpp>

using namespace nil::mtl;
using namespace nil::mtl::network;

BOOST_AUTO_TEST_CASE(serialization) {
    basp::header x {basp::message_type::handshake, 42, 4};
    std::vector<byte> buf;
    {
        binary_serializer sink {nullptr, buf};
        BOOST_CHECK_EQUAL(sink(x), none);
    }
    BOOST_CHECK_EQUAL(buf.size(), basp::header_size);
    auto buf2 = to_bytes(x);
    BOOST_REQUIRE_EQUAL(buf.size(), buf2.size());
    BOOST_CHECK(std::equal(buf.begin(), buf.end(), buf2.begin()));
    basp::header y;
    {
        binary_deserializer source {nullptr, buf};
        BOOST_CHECK_EQUAL(source(y), none);
    }
    BOOST_CHECK_EQUAL(x, y);
    auto z = basp::header::from_bytes(buf);
    BOOST_CHECK_EQUAL(x, z);
    BOOST_CHECK_EQUAL(y, z);
}

BOOST_AUTO_TEST_CASE(to_string) {
    basp::header x {basp::message_type::handshake, 42, 4};
    BOOST_CHECK_EQUAL(to_string(x), "basp::header(handshake, 42, 4)");
}
