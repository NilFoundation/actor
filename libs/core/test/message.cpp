//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt or
// http://opensource.org/licenses/BSD-3-Clause
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE message_operations_test

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <map>
#include <vector>
#include <string>
#include <numeric>
#include <iostream>

#include <set>
#include <unordered_set>

#include <nil/mtl/config.hpp>
#include <nil/mtl/all.hpp>

using std::make_tuple;
using std::map;
using std::string;
using std::vector;

using namespace nil::mtl;

BOOST_AUTO_TEST_CASE(apply_test) {
    auto f1 = [] { BOOST_ERROR("f1 invoked!"); };
    auto f2 = [](int i) { BOOST_CHECK_EQUAL(i, 42); };
    auto m = make_message(42);
    m.apply(f1);
    m.apply(f2);
}

BOOST_AUTO_TEST_CASE(drop_test) {
    auto m1 = make_message(1, 2, 3, 4, 5);
    std::vector<message> messages {
        m1, make_message(2, 3, 4, 5), make_message(3, 4, 5), make_message(4, 5), make_message(5), message {}};
    for (size_t i = 0; i < messages.size(); ++i) {
        BOOST_CHECK_EQUAL(to_string(m1.drop(i)), to_string(messages[i]));
    }
}

BOOST_AUTO_TEST_CASE(slice_test) {
    auto m1 = make_message(1, 2, 3, 4, 5);
    auto m2 = m1.slice(2, 2);
    BOOST_CHECK_EQUAL(to_string(m2), to_string(make_message(3, 4)));
}

BOOST_AUTO_TEST_CASE(extract1_test) {
    auto m1 = make_message(1.0, 2.0, 3.0);
    auto m2 = make_message(1, 2, 1.0, 2.0, 3.0);
    auto m3 = make_message(1.0, 1, 2, 2.0, 3.0);
    auto m4 = make_message(1.0, 2.0, 1, 2, 3.0);
    auto m5 = make_message(1.0, 2.0, 3.0, 1, 2);
    auto m6 = make_message(1, 2, 1.0, 2.0, 3.0, 1, 2);
    auto m7 = make_message(1.0, 1, 2, 3, 4, 2.0, 3.0);
    message_handler f {[](int, int) {}, [](float, float) {}};
    auto m1s = to_string(m1);
    BOOST_CHECK_EQUAL(to_string(m2.extract(f)), m1s);
    BOOST_CHECK_EQUAL(to_string(m3.extract(f)), m1s);
    BOOST_CHECK_EQUAL(to_string(m4.extract(f)), m1s);
    BOOST_CHECK_EQUAL(to_string(m5.extract(f)), m1s);
    BOOST_CHECK_EQUAL(to_string(m6.extract(f)), m1s);
    BOOST_CHECK_EQUAL(to_string(m7.extract(f)), m1s);
}

BOOST_AUTO_TEST_CASE(extract2_test) {
    auto m1 = make_message(1);
    BOOST_CHECK(m1.extract([](int) {}).empty());
    auto m2 = make_message(1.0, 2, 3, 4.0);
    auto m3 = m2.extract({[](int, int) {}, [](double, double) {}});
    // check for false positives through collapsing
    BOOST_CHECK_EQUAL(to_string(m3), to_string(make_message(1.0, 4.0)));
}

BOOST_AUTO_TEST_CASE(type_token_test) {
    auto m1 = make_message(get_atom::value);
    BOOST_CHECK_EQUAL(m1.type_token(), make_type_token<get_atom>());
}

BOOST_AUTO_TEST_CASE(concat_test) {
    auto m1 = make_message(get_atom::value);
    auto m2 = make_message(uint32_t {1});
    auto m3 = message::concat(m1, m2);
    BOOST_CHECK_EQUAL(to_string(m3), to_string(m1 + m2));
    BOOST_CHECK_EQUAL(to_string(m3), "('get', 1)");
    auto m4 = make_message(get_atom::value, uint32_t {1}, get_atom::value, uint32_t {1});
    BOOST_CHECK_EQUAL(to_string(message::concat(m3, message {}, m1, m2)), to_string(m4));
}

namespace {

    struct s1 {
        int value[3] = {10, 20, 30};
    };

    template<class Inspector>
    typename Inspector::result_type inspect(Inspector &f, s1 &x) {
        return f(x.value);
    }

    struct s2 {
        int value[4][2] = {{1, 10}, {2, 20}, {3, 30}, {4, 40}};
    };

    template<class Inspector>
    typename Inspector::result_type inspect(Inspector &f, s2 &x) {
        return f(x.value);
    }

    struct s3 {
        std::array<int, 4> value;

        s3() {
            std::iota(value.begin(), value.end(), 1);
        }
    };

    template<class Inspector>
    typename Inspector::result_type inspect(Inspector &f, s3 &x) {
        return f(x.value);
    }

    template<class... Ts>
    std::string msg_as_string(Ts &&... xs) {
        return to_string(make_message(std::forward<Ts>(xs)...));
    }

}    // namespace

BOOST_AUTO_TEST_CASE(compare_custom_types_test) {
    s2 tmp;
    tmp.value[0][1] = 100;
    BOOST_CHECK_NE(to_string(make_message(s2 {})), to_string(make_message(tmp)));
}

BOOST_AUTO_TEST_CASE(empty_to_string_test) {
    message msg;
    BOOST_CHECK((to_string(msg), "<empty-message>"));
}

BOOST_AUTO_TEST_CASE(integers_to_string_test) {
    using ivec = vector<int>;
    BOOST_CHECK_EQUAL(msg_as_string(1, 2, 3), "(1, 2, 3)");
    BOOST_CHECK_EQUAL(msg_as_string(ivec {1, 2, 3}), "([1, 2, 3])");
    BOOST_CHECK_EQUAL(msg_as_string(ivec {1, 2}, 3, 4, ivec {5, 6, 7}), "([1, 2], 3, 4, [5, 6, 7])");
}

BOOST_AUTO_TEST_CASE(strings_to_string_test) {
    using svec = vector<string>;
    auto msg1 = make_message("one", "two", "three");
    BOOST_CHECK_EQUAL(to_string(msg1), R"__(("one", "two", "three"))__");
    auto msg2 = make_message(svec {"one", "two", "three"});
    BOOST_CHECK_EQUAL(to_string(msg2), R"__((["one", "two", "three"]))__");
    auto msg3 = make_message(svec {"one", "two"}, "three", "four", svec {"five", "six", "seven"});
    BOOST_CHECK(to_string(msg3) == R"__((["one", "two"], "three", "four", ["five", "six", "seven"]))__");
    auto msg4 = make_message(R"(this is a "test")");
    BOOST_CHECK_EQUAL(to_string(msg4), "(\"this is a \\\"test\\\"\")");
}

BOOST_AUTO_TEST_CASE(maps_to_string_test) {
    std::map<int, int> m1{{1, 10}, {2, 20}, {3, 30}};
    auto msg1 = make_message(move(m1));
    BOOST_CHECK_EQUAL(to_string(msg1), "({1 = 10, 2 = 20, 3 = 30})");
}

BOOST_AUTO_TEST_CASE(tuples_to_string_test) {
    auto msg1 = make_message(make_tuple(1, 2, 3), 4, 5);
    BOOST_CHECK_EQUAL(to_string(msg1), "((1, 2, 3), 4, 5)");
    auto msg2 = make_message(make_tuple(string {"one"}, 2, uint32_t {3}), 4, true);
    BOOST_CHECK_EQUAL(to_string(msg2), "((\"one\", 2, 3), 4, true)");
}

BOOST_AUTO_TEST_CASE(arrays_to_string_test) {
    BOOST_CHECK_EQUAL(msg_as_string(s1{}), "([10, 20, 30])");
    auto msg2 = make_message(s2{});
    s2 tmp;
    tmp.value[0][1] = 100;
    BOOST_CHECK_EQUAL(to_string(msg2), "([[1, 10], [2, 20], [3, 30], [4, 40]])");
    BOOST_CHECK_EQUAL(msg_as_string(s3{}), "([1, 2, 3, 4])");
}
