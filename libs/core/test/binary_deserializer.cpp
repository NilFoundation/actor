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

#define BOOST_TEST_MODULE binary_deserializer_test

#include <nil/mtl/test/dsl.hpp>

#include <nil/mtl/serialization/binary_deserializer.hpp>

#include <cstring>
#include <vector>

#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/actor_system_config.hpp>
#include <nil/mtl/byte.hpp>
#include <nil/mtl/byte_buffer.hpp>
#include <nil/mtl/duration.hpp>
#include <nil/mtl/timestamp.hpp>

using namespace nil::mtl;

namespace {

    byte operator"" _b(unsigned long long int x) {
        return static_cast<byte>(x);
    }

    byte operator"" _b(char x) {
        return static_cast<byte>(x);
    }

    enum class test_enum : int32_t {
        a,
        b,
        c,
    };

    struct test_data {
        int32_t i32_;
        int64_t i64_;
        float f32_;
        double f64_;
        nil::mtl::duration dur_;
        nil::mtl::timestamp ts_;
        test_enum te_;
        std::string str_;
    };

    bool operator==(const test_data &data, const test_data &other) {
        return (data.f64_ == other.f64_ && data.i32_ == other.i32_ && data.i64_ == other.i64_ &&
                data.str_ == other.str_ && data.te_ == other.te_ && data.ts_ == other.ts_);
    }

    template<class Inspector>
    typename Inspector::result_type inspect(Inspector &f, test_data &x) {
        return f(nil::mtl::meta::type_name("test_data"), x.i32_, x.i64_, x.f32_, x.f64_, x.dur_, x.ts_, x.te_, x.str_);
    }

    struct fixture {
        template<class T>
        auto load(const std::vector<byte> &buf) {
            auto result = T {};
            binary_deserializer source {nullptr, buf};
            if (auto err = source(result))
                BOOST_FAIL("binary_deserializer failed to load: " << actor_system_config::render(err));
            return result;
        }

        template<class... Ts>
        void load(const std::vector<byte> &buf, Ts &... xs) {
            binary_deserializer source {nullptr, buf};
            if (auto err = source(xs...))
                BOOST_FAIL("binary_deserializer failed to load: " << actor_system_config::render(err));
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(binary_deserializer_tests, fixture)

#define SUBTEST(msg)         \
    BOOST_TEST_MESSAGE(msg); \
    for (int subtest_dummy = 0; subtest_dummy < 1; ++subtest_dummy)

#define CHECK_EQ(lhs, rhs) BOOST_CHECK_EQUAL(lhs, rhs)

#define CHECK_LOAD(type, value, ...) BOOST_CHECK(load<type>({__VA_ARGS__}) == value)

BOOST_AUTO_TEST_CASE(binary_deserializer_handles_all_primitive_types) {
    SUBTEST("8-bit integers") {
        CHECK_LOAD(int8_t, 60, 0b00111100_b);
        CHECK_LOAD(int8_t, -61, 0b11000011_b);
        CHECK_LOAD(uint8_t, 60u, 0b00111100_b);
        CHECK_LOAD(uint8_t, 195u, 0b11000011_b);
    }
    SUBTEST("16-bit integers") {
        CHECK_LOAD(int16_t, 85, 0b00000000_b, 0b01010101_b);
        CHECK_LOAD(int16_t, -32683, 0b10000000_b, 0b01010101_b);
        CHECK_LOAD(uint16_t, 85u, 0b00000000_b, 0b01010101_b);
        CHECK_LOAD(uint16_t, 32853u, 0b10000000_b, 0b01010101_b);
    }
    SUBTEST("32-bit integers") {
        CHECK_LOAD(int32_t, -345, 0xFF_b, 0xFF_b, 0xFE_b, 0xA7_b);
        CHECK_LOAD(uint32_t, 4294966951u, 0xFF_b, 0xFF_b, 0xFE_b, 0xA7_b);
    }
    SUBTEST("64-bit integers") {
        CHECK_LOAD(int64_t, -1234567890123456789ll,    //
                   0xEE_b, 0xDD_b, 0xEF_b, 0x0B_b, 0x82_b, 0x16_b, 0x7E_b, 0xEB_b);
        CHECK_LOAD(uint64_t, 17212176183586094827llu,    //
                   0xEE_b, 0xDD_b, 0xEF_b, 0x0B_b, 0x82_b, 0x16_b, 0x7E_b, 0xEB_b);
    }
    SUBTEST("floating points use IEEE-754 conversion") {
        CHECK_LOAD(float, 3.45f, 0x40_b, 0x5C_b, 0xCC_b, 0xCD_b);
    }
    SUBTEST("strings use a varbyte-encoded size prefix") {
        CHECK_LOAD(std::string, "hello", 5_b, 'h'_b, 'e'_b, 'l'_b, 'l'_b, 'o'_b);
    }
    SUBTEST("enum types") {
        CHECK_LOAD(test_enum, test_enum::a, 0_b, 0_b, 0_b, 0_b);
        CHECK_LOAD(test_enum, test_enum::b, 0_b, 0_b, 0_b, 1_b);
        CHECK_LOAD(test_enum, test_enum::c, 0_b, 0_b, 0_b, 2_b);
    }
}

BOOST_AUTO_TEST_CASE(concatenation) {
    SUBTEST("calling f(a, b) writes a and b into the buffer in order") {
        int8_t x = 0;
        int16_t y = 0;
        load(byte_buffer({7_b, 0x80_b, 0x55_b}), x, y);
        BOOST_CHECK_EQUAL(x, 7);
        BOOST_CHECK_EQUAL(y, -32683);
        load(byte_buffer({0x80_b, 0x55_b, 7_b}), y, x);
        load(byte_buffer({7_b, 0x80_b, 0x55_b}), x, y);
    }
    SUBTEST("calling f(make_pair(a, b)) is equal to calling f(a, b)") {
        using i8i16_pair = std::pair<int8_t, int16_t>;
        using i16i8_pair = std::pair<int16_t, int8_t>;
        CHECK_LOAD(i8i16_pair, std::make_pair(int8_t {7}, int16_t {-32683}),    //
                   7_b, 0x80_b, 0x55_b);
        CHECK_LOAD(i16i8_pair, std::make_pair(int16_t {-32683}, int8_t {7}),    //
                   0x80_b, 0x55_b, 7_b);
    }
    SUBTEST("calling f(make_tuple(a, b)) is equivalent to f(make_pair(a, b))") {
        using i8i16_tuple = std::tuple<int8_t, int16_t>;
        using i16i8_tuple = std::tuple<int16_t, int8_t>;
        CHECK_LOAD(i8i16_tuple, std::make_tuple(int8_t {7}, int16_t {-32683}),    //
                   7_b, 0x80_b, 0x55_b);
        CHECK_LOAD(i16i8_tuple, std::make_tuple(int16_t {-32683}, int8_t {7}),    //
                   0x80_b, 0x55_b, 7_b);
    }
    SUBTEST("arrays behave like tuples") {
        int8_t xs[] = {0, 0, 0};
        load(byte_buffer({1_b, 2_b, 3_b}), xs);
        BOOST_CHECK_EQUAL(xs[0], 1);
        BOOST_CHECK_EQUAL(xs[1], 2);
        BOOST_CHECK_EQUAL(xs[2], 3);
    }
}

BOOST_AUTO_TEST_CASE(container_types) {
    SUBTEST("STL vectors") {
        CHECK_LOAD(std::vector<int8_t>, std::vector<int8_t>({1, 2, 4, 8}),    //
                   4_b, 1_b, 2_b, 4_b, 8_b);
    }
    SUBTEST("STL sets") {
        CHECK_LOAD(std::set<int8_t>, std::set<int8_t>({1, 2, 4, 8}),    //
                   4_b, 1_b, 2_b, 4_b, 8_b);
    }
}

BOOST_AUTO_TEST_CASE(binary_serializer_picks_up_inspect_functions) {
    SUBTEST("duration") {
        CHECK_LOAD(duration, duration(time_unit::minutes, 3),
                   // Bytes 1-4 contain the time_unit.
                   0_b, 0_b, 0_b, 1_b,
                   // Bytes 5-12 contain the count.
                   0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 3_b);
    }
    SUBTEST("node ID") {
        auto nid = make_node_id(123, "000102030405060708090A0B0C0D0E0F10111213");
        CHECK_LOAD(node_id, unbox(nid),
                   // Implementation ID: atom("default").
                   0_b, 0_b, 0x3E_b, 0x9A_b, 0xAB_b, 0x9B_b, 0xAC_b, 0x79_b,
                   // Process ID.
                   0_b, 0_b, 0_b, 123_b,
                   // Host ID.
                   0_b, 1_b, 2_b, 3_b, 4_b, 5_b, 6_b, 7_b, 8_b, 9_b,    //
                   10_b, 11_b, 12_b, 13_b, 14_b, 15_b, 16_b, 17_b, 18_b, 19_b);
    }
    SUBTEST("custom struct") {
        test_data value {-345,
                         -1234567890123456789ll,
                         3.45,
                         54.3,
                         nil::mtl::duration(nil::mtl::time_unit::seconds, 123),
                         nil::mtl::timestamp {nil::mtl::timestamp::duration {1478715821 * 1000000000ll}},
                         test_enum::b,
                         "Lorem ipsum dolor sit amet."};
        CHECK_LOAD(test_data, value,
                   // 32-bit i32_ member: -345
                   0xFF_b, 0xFF_b, 0xFE_b, 0xA7_b,
                   // 64-bit i64_ member: -1234567890123456789ll
                   0xEE_b, 0xDD_b, 0xEF_b, 0x0B_b, 0x82_b, 0x16_b, 0x7E_b, 0xEB_b,
                   // 32-bit f32_ member: 3.45f
                   0x40_b, 0x5C_b, 0xCC_b, 0xCD_b,
                   // 64-bit f64_ member: 54.3
                   0x40_b, 0x4B_b, 0x26_b, 0x66_b, 0x66_b, 0x66_b, 0x66_b, 0x66_b,
                   // 32-bit dur_.unit member: time_unit::seconds
                   0x00_b, 0x00_b, 0x00_b, 0x02_b,
                   // 64-bit dur_.count member: 123
                   0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x7B_b,
                   // 64-bit ts_ member.
                   0x14_b, 0x85_b, 0x74_b, 0x34_b, 0x62_b, 0x74_b, 0x82_b, 0x00_b,
                   // 32-bit te_ member: test_enum::b
                   0x00_b, 0x00_b, 0x00_b, 0x01_b,
                   // str_ member:
                   0x1B_b,    //
                   'L'_b, 'o'_b, 'r'_b, 'e'_b, 'm'_b, ' '_b, 'i'_b, 'p'_b, 's'_b, 'u'_b, 'm'_b, ' '_b, 'd'_b, 'o'_b,
                   'l'_b, 'o'_b, 'r'_b, ' '_b, 's'_b, 'i'_b, 't'_b, ' '_b, 'a'_b, 'm'_b, 'e'_b, 't'_b, '.'_b);
    }
}

BOOST_AUTO_TEST_SUITE_END()
