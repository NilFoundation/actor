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

#define BOOST_TEST_MODULE serialization_test

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <nil/mtl/config.hpp>

#include <new>
#include <set>
#include <list>
#include <stack>
#include <tuple>
#include <locale>
#include <memory>
#include <string>
#include <limits>
#include <vector>
#include <cstring>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <iterator>
#include <typeinfo>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <type_traits>

#include <nil/mtl/message.hpp>
#include <nil/mtl/streambuf.hpp>
#include <nil/mtl/serializer.hpp>
#include <nil/mtl/ref_counted.hpp>
#include <nil/mtl/serialization/deserializer.hpp>
#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/proxy_registry.hpp>
#include <nil/mtl/message_handler.hpp>
#include <nil/mtl/event_based_actor.hpp>
#include <nil/mtl/primitive_variant.hpp>
#include <nil/mtl/actor_system_config.hpp>
#include <nil/mtl/make_type_erased_view.hpp>
#include <nil/mtl/make_type_erased_tuple_view.hpp>

#include <nil/mtl/serialization/binary_serializer.hpp>
#include <nil/mtl/serialization/binary_deserializer.hpp>
#include <nil/mtl/serialization/stream_serializer.hpp>
#include <nil/mtl/serialization/stream_deserializer.hpp>

#include <nil/mtl/detail/ieee_754.hpp>
#include <nil/mtl/detail/int_list.hpp>
#include <nil/mtl/detail/safe_equal.hpp>
#include <nil/mtl/detail/type_traits.hpp>
#include <nil/mtl/detail/enum_to_string.hpp>
#include <nil/mtl/detail/get_mac_addresses.hpp>

using namespace std;
using namespace nil::mtl;
using nil::mtl::detail::type_erased_value_impl;

namespace {

    using strmap = map<string, u16string>;

    struct raw_struct {
        string str;
    };

    template<class Inspector>
    typename Inspector::result_type inspect(Inspector &f, raw_struct &x) {
        return f(x.str);
    }

    bool operator==(const raw_struct &lhs, const raw_struct &rhs) {
        return lhs.str == rhs.str;
    }

    enum class test_enum : uint32_t { a, b, c };

    const char *test_enum_strings[] = {"a", "b", "c"};

    std::string to_string(test_enum x) {
        return detail::enum_to_string(x, test_enum_strings);
    }

    struct test_array {
        int value[4];
        int value2[2][4];
    };

    template<class Inspector>
    typename Inspector::result_type inspect(Inspector &f, test_array &x) {
        return f(x.value, x.value2);
    }

    struct test_empty_non_pod {
        test_empty_non_pod() = default;

        test_empty_non_pod(const test_empty_non_pod &) = default;

        test_empty_non_pod &operator=(const test_empty_non_pod &) = default;

        virtual void foo() {
            // nop
        }

        virtual ~test_empty_non_pod() {
            // nop
        }
    };

    template<class Inspector>
    typename Inspector::result_type inspect(Inspector &f, test_empty_non_pod &) {
        return f();
    }

    class config : public actor_system_config {
    public:
        config() {
            add_message_type<test_enum>("test_enum");
            add_message_type<raw_struct>("raw_struct");
            add_message_type<test_array>("test_array");
            add_message_type<test_empty_non_pod>("test_empty_non_pod");
            add_message_type<std::vector<bool>>("bool_vector");
        }
    };

    struct fixture {
        int32_t i32 = -345;
        int64_t i64 = -1234567890123456789ll;
        float f32 = 3.45f;
        double f64 = 54.3;
        duration dur = duration {time_unit::seconds, 123};
        timestamp ts = timestamp {timestamp::duration {1478715821 * 1000000000ll}};
        test_enum te = test_enum::b;
        string str = "Lorem ipsum dolor sit amet.";
        raw_struct rs;
        test_array ta {
            {0, 1, 2, 3},
            {{0, 1, 2, 3}, {4, 5, 6, 7}},
        };
        int ra[3] = {1, 2, 3};

        config cfg;
        actor_system system;
        scoped_execution_unit context;
        message msg;

        template<class T, class... Ts>
        vector<char> serialize(T &x, Ts &... xs) {
            vector<char> buf;
            binary_serializer bs {&context, buf};
            bs(x, xs...);
            return buf;
        }

        template<class T, class... Ts>
        void deserialize(const vector<char> &buf, T &x, Ts &... xs) {
            binary_deserializer bd {&context, buf};
            bd(x, xs...);
        }

        // serializes `x` and then deserializes and returns the serialized value
        template<class T>
        T roundtrip(T x) {
            T result;
            deserialize(serialize(x), result);
            return result;
        }

        // converts `x` to a message, serialize it, then deserializes it, and
        // finally returns unboxed value
        template<class T>
        T msg_roundtrip(const T &x) {
            message result;
            auto tmp = make_message(x);
            deserialize(serialize(tmp), result);
            BOOST_REQUIRE(result.match_elements<T>());
            return result.get_as<T>(0);
        }

        fixture() : system(cfg), context(&system) {
            rs.str.assign(string(str.rbegin(), str.rend()));
            msg = make_message(i32, i64, dur, ts, te, str, rs);
        }
    };

    struct is_message {
        explicit is_message(message &msgref) : msg(msgref) {
            // nop
        }

        message &msg;

        template<class T, class... Ts>
        bool equal(T &&v, Ts &&... vs) {
            bool ok = false;
            // work around for gcc 4.8.4 bug
            auto tup = tie(v, vs...);
            message_handler impl {[&](T const &u, Ts const &... us) { ok = tup == tie(u, us...); }};
            impl(msg);
            return ok;
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(serialization_tests, fixture)

BOOST_AUTO_TEST_CASE(ieee_754_conversion_test) {
    // check conversion of float
    float f1 = 3.1415925f;                      // float value
    auto p1 = nil::mtl::detail::pack754(f1);    // packet value
    BOOST_CHECK_EQUAL(p1, static_cast<decltype(p1)>(0x40490FDA));
    auto u1 = nil::mtl::detail::unpack754(p1);    // unpacked value
    BOOST_CHECK_EQUAL(f1, u1);
    // check conversion of double
    double f2 = 3.14159265358979311600;         // double value
    auto p2 = nil::mtl::detail::pack754(f2);    // packet value
    BOOST_CHECK_EQUAL(p2, static_cast<decltype(p2)>(0x400921FB54442D18));
    auto u2 = nil::mtl::detail::unpack754(p2);    // unpacked value
    BOOST_CHECK_EQUAL(f2, u2);
}

BOOST_AUTO_TEST_CASE(i32_values_test) {
    auto buf = serialize(i32);
    int32_t x;
    deserialize(buf, x);
    BOOST_CHECK_EQUAL(i32, x);
}

BOOST_AUTO_TEST_CASE(i64_values_test) {
    auto buf = serialize(i64);
    int64_t x;
    deserialize(buf, x);
    BOOST_CHECK_EQUAL(i64, x);
}

BOOST_AUTO_TEST_CASE(float_values_test) {
    auto buf = serialize(f32);
    float x;
    deserialize(buf, x);
    BOOST_CHECK_EQUAL(f32, x);
}

BOOST_AUTO_TEST_CASE(double_values_test) {
    auto buf = serialize(f64);
    double x;
    deserialize(buf, x);
    BOOST_CHECK_EQUAL(f64, x);
}

BOOST_AUTO_TEST_CASE(duration_values_test) {
    auto buf = serialize(dur);
    duration x;
    deserialize(buf, x);
    BOOST_CHECK(dur == x);
}

BOOST_AUTO_TEST_CASE(timestamp_values_test) {
    auto buf = serialize(ts);
    timestamp x;
    deserialize(buf, x);
    BOOST_CHECK(ts == x);
}

BOOST_AUTO_TEST_CASE(enum_classes_test) {
    auto buf = serialize(te);
    test_enum x;
    deserialize(buf, x);
    BOOST_CHECK(te == x);
}

BOOST_AUTO_TEST_CASE(strings_test) {
    auto buf = serialize(str);
    string x;
    deserialize(buf, x);
    BOOST_CHECK_EQUAL(str, x);
}

BOOST_AUTO_TEST_CASE(custom_struct_test) {
    auto buf = serialize(rs);
    raw_struct x;
    deserialize(buf, x);
    BOOST_CHECK(rs == x);
}

BOOST_AUTO_TEST_CASE(atoms_test) {
    auto foo = atom("foo");
    BOOST_CHECK(foo == roundtrip(foo));
    BOOST_CHECK(foo == msg_roundtrip(foo));
    using bar_atom = atom_constant<atom("bar")>;
    BOOST_CHECK(bar_atom::value == roundtrip(atom("bar")));
    BOOST_CHECK(bar_atom::value == msg_roundtrip(atom("bar")));
}

BOOST_AUTO_TEST_CASE(raw_arrays_test) {
    auto buf = serialize(ra);
    int x[3];
    deserialize(buf, x);
    for (auto i = 0; i < 3; ++i)
        BOOST_CHECK_EQUAL(ra[i], x[i]);
}

BOOST_AUTO_TEST_CASE(arrays_test) {
    auto buf = serialize(ta);
    test_array x;
    deserialize(buf, x);
    for (auto i = 0; i < 4; ++i)
        BOOST_CHECK_EQUAL(ta.value[i], x.value[i]);
    for (auto i = 0; i < 2; ++i) {
        for (auto j = 0; j < 4; ++j) {
            BOOST_CHECK_EQUAL(ta.value2[i][j], x.value2[i][j]);
        }
    }
}

BOOST_AUTO_TEST_CASE(empty_non_pods_test) {
    test_empty_non_pod x;
    auto buf = serialize(x);
    BOOST_REQUIRE(buf.empty());
    deserialize(buf, x);
}

std::string hexstr(const std::vector<char> &buf) {
    using namespace std;
    ostringstream oss;
    oss << hex;
    oss.fill('0');
    for (auto &c : buf) {
        oss.width(2);
        oss << int {c};
    }
    return oss.str();
}

BOOST_AUTO_TEST_CASE(messages_test) {
    // serialize original message which uses tuple_vals internally and
    // deserialize into a message which uses type_erased_value pointers
    message x;
    auto buf1 = serialize(msg);
    deserialize(buf1, x);
    BOOST_CHECK_EQUAL(to_string(msg), to_string(x));
    BOOST_CHECK(is_message(x).equal(i32, i64, dur, ts, te, str, rs));
    // serialize fully dynamic message again (do another roundtrip)
    message y;
    auto buf2 = serialize(x);
    BOOST_CHECK(buf1 == buf2);
    deserialize(buf2, y);
    BOOST_CHECK_EQUAL(to_string(msg), to_string(y));
    BOOST_CHECK(is_message(y).equal(i32, i64, dur, ts, te, str, rs));
}

BOOST_AUTO_TEST_CASE(multiple_messages_test) {
    auto m = make_message(rs, te);
    auto buf = serialize(te, m, msg);
    test_enum t;
    message m1;
    message m2;
    deserialize(buf, t, m1, m2);
    BOOST_CHECK(std::make_tuple(t, to_string(m1), to_string(m2)) == std::make_tuple(te, to_string(m), to_string(msg)));
    BOOST_CHECK(is_message(m1).equal(rs, te));
    BOOST_CHECK(is_message(m2).equal(i32, i64, dur, ts, te, str, rs));
}

BOOST_AUTO_TEST_CASE(type_erased_value_test) {
    auto buf = serialize(str);
    type_erased_value_ptr ptr {new type_erased_value_impl<std::string>};
    binary_deserializer bd {&context, buf.data(), buf.size()};
    ptr->load(bd);
    BOOST_CHECK_EQUAL(str, *reinterpret_cast<const std::string *>(ptr->get()));
}

BOOST_AUTO_TEST_CASE(type_erased_view_test) {
    auto str_view = make_type_erased_view(str);
    auto buf = serialize(str_view);
    std::string res;
    deserialize(buf, res);
    BOOST_CHECK_EQUAL(str, res);
}

BOOST_AUTO_TEST_CASE(type_erased_tuple_test) {
    auto tview = make_type_erased_tuple_view(str, i32);
    BOOST_CHECK_EQUAL(to_string(tview), deep_to_string(std::make_tuple(str, i32)));
    auto buf = serialize(tview);
    BOOST_REQUIRE(!buf.empty());
    std::string tmp1;
    int32_t tmp2;
    deserialize(buf, tmp1, tmp2);
    BOOST_CHECK_EQUAL(tmp1, str);
    BOOST_CHECK_EQUAL(tmp2, i32);
    deserialize(buf, tview);
    BOOST_CHECK_EQUAL(to_string(tview), deep_to_string(std::make_tuple(str, i32)));
}

BOOST_AUTO_TEST_CASE(streambuf_serialization_test) {
    auto data = std::string {"The quick brown fox jumps over the lazy dog"};
    std::vector<char> buf;
    // First, we check the standard use case in MTL where stream serializers own
    // their stream buffers.
    stream_serializer<vectorbuf> bs {vectorbuf {buf}};
    auto e = bs(data);
    BOOST_REQUIRE(e == none);
    stream_deserializer<charbuf> bd {charbuf {buf}};
    std::string target;
    e = bd(target);
    BOOST_REQUIRE(e == none);
    BOOST_CHECK_EQUAL(data, target);
    // Second, we test another use case where the serializers only keep
    // references of the stream buffers.
    buf.clear();
    target.clear();
    vectorbuf vb {buf};
    stream_serializer<vectorbuf &> vs {vb};
    e = vs(data);
    BOOST_REQUIRE(e == none);
    charbuf cb {buf};
    stream_deserializer<charbuf &> vd {cb};
    e = vd(target);
    BOOST_REQUIRE(e == none);
    BOOST_CHECK(data == target);
}

BOOST_AUTO_TEST_CASE(byte_sequence_optimization_test) {
    std::vector<uint8_t> data(42);
    std::fill(data.begin(), data.end(), 0x2a);
    std::vector<uint8_t> buf;
    using streambuf_type = containerbuf<std::vector<uint8_t>>;
    streambuf_type cb {buf};
    stream_serializer<streambuf_type &> bs {cb};
    auto e = bs(data);
    BOOST_REQUIRE(!e);
    data.clear();
    streambuf_type cb2 {buf};
    stream_deserializer<streambuf_type &> bd {cb2};
    e = bd(data);
    BOOST_REQUIRE(!e);
    BOOST_CHECK_EQUAL(data.size(), 42u);
    BOOST_CHECK(std::all_of(data.begin(), data.end(), [](uint8_t c) { return c == 0x2a; }));
}

BOOST_AUTO_TEST_CASE(long_sequences_test) {
    std::vector<char> data;
    binary_serializer sink {nullptr, data};
    size_t n = std::numeric_limits<uint32_t>::max();
    sink.begin_sequence(n);
    sink.end_sequence();
    binary_deserializer source {nullptr, data};
    size_t m = 0;
    source.begin_sequence(m);
    source.end_sequence();
    BOOST_CHECK_EQUAL(n, m);
}

// -- our vector<bool> serialization packs into an uint64_t. Hence, the
// critical sizes to test are 0, 1, 63, 64, and 65.

BOOST_AUTO_TEST_CASE(bool_vector_size_0_test) {
    std::vector<bool> xs;
    BOOST_CHECK_EQUAL(deep_to_string(xs), "[]");
    BOOST_CHECK(xs == roundtrip(xs));
    BOOST_CHECK(xs == msg_roundtrip(xs));
}

BOOST_AUTO_TEST_CASE(bool_vector_size_1_test) {
    std::vector<bool> xs {true};
    BOOST_CHECK_EQUAL(deep_to_string(xs), "[true]");
    BOOST_CHECK(xs == roundtrip(xs));
    BOOST_CHECK(xs == msg_roundtrip(xs));
}

BOOST_AUTO_TEST_CASE(bool_vector_size_63_test) {
    std::vector<bool> xs;
    for (int i = 0; i < 63; ++i) {
        xs.push_back(i % 3 == 0);
    }
    BOOST_CHECK_EQUAL(deep_to_string(xs),
        "[true, false, false, true, false, false, true, false, false, true, false, "
        "false, true, false, false, true, false, false, true, false, false, true, "
        "false, false, true, false, false, true, false, false, true, false, false, "
        "true, false, false, true, false, false, true, false, false, true, false, "
        "false, true, false, false, true, false, false, true, false, false, true, "
        "false, false, true, false, false, true, false, false]");
    BOOST_CHECK(xs == roundtrip(xs));
    BOOST_CHECK(xs == msg_roundtrip(xs));
}

BOOST_AUTO_TEST_CASE(bool_vector_size_64_test) {
    std::vector<bool> xs;
    for (int i = 0; i < 64; ++i) {
        xs.push_back(i % 5 == 0);
    }
    BOOST_CHECK_EQUAL(deep_to_string(xs),
        "[true, false, false, false, false, true, false, false, "
        "false, false, true, false, false, false, false, true, "
        "false, false, false, false, true, false, false, false, "
        "false, true, false, false, false, false, true, false, "
        "false, false, false, true, false, false, false, false, "
        "true, false, false, false, false, true, false, false, "
        "false, false, true, false, false, false, false, true, "
        "false, false, false, false, true, false, false, false]");
    BOOST_CHECK(xs == roundtrip(xs));
    BOOST_CHECK(xs == msg_roundtrip(xs));
}

BOOST_AUTO_TEST_CASE(bool_vector_size_65_test) {
    std::vector<bool> xs;
    for (int i = 0; i < 65; ++i) {
        xs.push_back(!(i % 7 == 0));
    }
    BOOST_CHECK_EQUAL(deep_to_string(xs),
        "[false, true, true, true, true, true, true, false, true, true, true, "
        "true, true, true, false, true, true, true, true, true, true, false, true, "
        "true, true, true, true, true, false, true, true, true, true, true, true, "
        "false, true, true, true, true, true, true, false, true, true, true, true, "
        "true, true, false, true, true, true, true, true, true, false, true, true, "
        "true, true, true, true, false, true]");
    BOOST_CHECK(xs == roundtrip(xs));
    BOOST_CHECK(xs == msg_roundtrip(xs));
}

BOOST_AUTO_TEST_SUITE_END()
