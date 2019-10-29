//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt for Boost License or
// http://opensource.org/licenses/BSD-3-Clause for BSD 3-Clause License
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE to_string_test

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <nil/mtl/config.hpp>
#include <nil/mtl/to_string.hpp>

using namespace std;
using namespace nil::mtl;

BOOST_AUTO_TEST_CASE(buffer_test) {
    // Use `signed char` explicitly to make sure all compilers agree.
    std::vector<signed char> buf;
    BOOST_CHECK_EQUAL(deep_to_string(buf), "[]");
    BOOST_CHECK_EQUAL(deep_to_string(meta::hex_formatted(), buf), "00");
    buf.push_back(-1);
    BOOST_CHECK_EQUAL(deep_to_string(buf), "[-1]");
    BOOST_CHECK_EQUAL(deep_to_string(meta::hex_formatted(), buf), "FF");
    buf.push_back(0);
    BOOST_CHECK_EQUAL(deep_to_string(buf), "[-1, 0]");
    BOOST_CHECK_EQUAL(deep_to_string(meta::hex_formatted(), buf), "FF00");
    buf.push_back(127);
    BOOST_CHECK_EQUAL(deep_to_string(buf), "[-1, 0, 127]");
    BOOST_CHECK_EQUAL(deep_to_string(meta::hex_formatted(), buf), "FF007F");
    buf.push_back(10);
    BOOST_CHECK_EQUAL(deep_to_string(buf), "[-1, 0, 127, 10]");
    BOOST_CHECK_EQUAL(deep_to_string(meta::hex_formatted(), buf), "FF007F0A");
    buf.push_back(16);
    BOOST_CHECK_EQUAL(deep_to_string(buf), "[-1, 0, 127, 10, 16]");
    BOOST_CHECK_EQUAL(deep_to_string(meta::hex_formatted(), buf), "FF007F0A10");
}
