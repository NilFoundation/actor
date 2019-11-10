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

#include <nil/mtl/config.hpp>

#define BOOST_TEST_MODULE extract_test

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <string>
#include <vector>

#include <nil/mtl/all.hpp>

using namespace nil::mtl;

using std::string;

BOOST_AUTO_TEST_CASE(type_sequences_test) {
    auto _64 = uint64_t {64};
    std::string str = "str";
    auto msg = make_message(1.0, 2.f, str, 42, _64);
    auto df = [](double, float) {};
    auto fs = [](float, const string &) {};
    auto iu = [](int, uint64_t) {};
    BOOST_CHECK_EQUAL(to_string(msg.extract(df)), to_string(make_message(str, 42, _64)));
    BOOST_CHECK_EQUAL(to_string(msg.extract(fs)), to_string(make_message(1.0, 42, _64)));
    BOOST_CHECK_EQUAL(to_string(msg.extract(iu)), to_string(make_message(1.0, 2.f, str)));
}