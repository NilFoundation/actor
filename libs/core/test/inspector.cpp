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

#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include <nil/mtl/config.hpp>

#define BOOST_TEST_MODULE inspector_test

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/type_erased_value.hpp>
#include <nil/mtl/actor_system_config.hpp>
#include <nil/mtl/make_type_erased_value.hpp>

#include <nil/mtl/serialization/binary_serializer.hpp>
#include <nil/mtl/serialization/binary_deserializer.hpp>

#include <nil/mtl/detail/stringification_inspector.hpp>

using namespace nil::mtl;

namespace {

    template<class T>
    class inspector_adapter_base {
    public:
        inspector_adapter_base(T &impl) : impl_(impl) {
            // nop
        }

    protected:
        T &impl_;
    };

    template<class RoundtripPolicy>
    struct check_impl {
        RoundtripPolicy &p_;

        template<class T>
        bool operator()(T x) {
            return p_(x);
        }
    };

    template<class T>
    using nl = std::numeric_limits<T>;

    // an empty type
    struct dummy_tag_type {};

    constexpr bool operator==(dummy_tag_type, dummy_tag_type) {
        return true;
    }

    // a POD type
    struct dummy_struct {
        int a;
        std::string b;
    };

    bool operator==(const dummy_struct &x, const dummy_struct &y) {
        return x.a == y.a && x.b == y.b;
    }

    template<class Inspector>
    typename Inspector::result_type inspect(Inspector &f, dummy_struct &x) {
        return f(meta::type_name("dummy_struct"), x.a, x.b);
    }

    // two different styles of enums
    enum dummy_enum { de_foo, de_bar };

    enum dummy_enum_class : short { foo, bar };

    std::string to_string(dummy_enum_class x) {
        return x == dummy_enum_class::foo ? "foo" : "bar";
    }

    template<class Policy>
    void test_impl(Policy &p) {
        check_impl<Policy> check {p};
        // check primitive types
        BOOST_CHECK(check(true));
        BOOST_CHECK(check(false));
        BOOST_CHECK(check(nl<int8_t>::lowest()));
        BOOST_CHECK(check(nl<int8_t>::max()));
        BOOST_CHECK(check(nl<uint8_t>::lowest()));
        BOOST_CHECK(check(nl<uint8_t>::max()));
        BOOST_CHECK(check(nl<int16_t>::lowest()));
        BOOST_CHECK(check(nl<int16_t>::max()));
        BOOST_CHECK(check(nl<uint16_t>::lowest()));
        BOOST_CHECK(check(nl<uint16_t>::max()));
        BOOST_CHECK(check(nl<int32_t>::lowest()));
        BOOST_CHECK(check(nl<int32_t>::max()));
        BOOST_CHECK(check(nl<uint32_t>::lowest()));
        BOOST_CHECK(check(nl<uint32_t>::max()));
        BOOST_CHECK(check(nl<int64_t>::lowest()));
        BOOST_CHECK(check(nl<int64_t>::max()));
        BOOST_CHECK(check(nl<uint64_t>::lowest()));
        BOOST_CHECK(check(nl<uint64_t>::max()));
        BOOST_CHECK(check(nl<float>::lowest()));
        BOOST_CHECK(check(nl<float>::max()));
        BOOST_CHECK(check(nl<double>::lowest()));
        BOOST_CHECK(check(nl<double>::max()));
        BOOST_CHECK(check(nl<long double>::lowest()));
        BOOST_CHECK(check(nl<long double>::max()));
        // atoms
        BOOST_CHECK(check(atom("")));
        BOOST_CHECK(check(atom("0123456789")));
        // various containers
        BOOST_CHECK(check(std::array<int, 3> {{1, 2, 3}}));
        BOOST_CHECK(check(std::vector<char> {}));
        BOOST_CHECK(check(std::vector<char> {1, 2, 3}));
        BOOST_CHECK(check(std::vector<int> {}));
        BOOST_CHECK(check(std::vector<int> {1, 2, 3}));
        BOOST_CHECK(check(std::list<int> {}));
        BOOST_CHECK(check(std::list<int> {1, 2, 3}));
        BOOST_CHECK(check(std::set<int> {}));
        BOOST_CHECK(check(std::set<int> {1, 2, 3}));
        BOOST_CHECK(check(std::unordered_set<int> {}));
        BOOST_CHECK(check(std::unordered_set<int> {1, 2, 3}));
        BOOST_CHECK(check(std::map<int, int> {}));
        BOOST_CHECK(check(std::map<int, int> {{1, 1}, {2, 2}, {3, 3}}));
        BOOST_CHECK(check(std::unordered_map<int, int> {}));
        BOOST_CHECK(check(std::unordered_map<int, int> {{1, 1}, {2, 2}, {3, 3}}));
        // user-defined types
        BOOST_CHECK(check(dummy_struct {10, "hello"}));
        // optionals
        BOOST_CHECK(check(optional<int> {}));
        BOOST_CHECK(check(optional<int> {42}));
        // strings
        BOOST_CHECK(check(std::string {}));
        BOOST_CHECK(check(std::string {""}));
        BOOST_CHECK(check(std::string {"test"}));
        BOOST_CHECK(check(std::u16string {}));
        BOOST_CHECK(check(std::u16string {u""}));
        BOOST_CHECK(check(std::u16string {u"test"}));
        BOOST_CHECK(check(std::u32string {}));
        BOOST_CHECK(check(std::u32string {U""}));
        BOOST_CHECK(check(std::u32string {U"test"}));
        // enums
        BOOST_CHECK(check(de_foo));
        BOOST_CHECK(check(de_bar));
        BOOST_CHECK(check(dummy_enum_class::foo));
        BOOST_CHECK(check(dummy_enum_class::bar));
        // empty type
        BOOST_CHECK(check(dummy_tag_type {}));
        // pair and tuple
        BOOST_CHECK(check(std::make_pair(std::string("hello"), 42)));
        BOOST_CHECK(check(std::make_pair(std::make_pair(1, 2), 3)));
        BOOST_CHECK(check(std::make_pair(std::make_tuple(1, 2), 3)));
        BOOST_CHECK(check(std::make_tuple(1, 2, 3, 4)));
        BOOST_CHECK(check(std::make_tuple(std::make_tuple(1, 2, 3), 4)));
        BOOST_CHECK(check(std::make_tuple(std::make_pair(1, 2), 3, 4)));
        // variant<>
        BOOST_CHECK(check(variant<none_t> {}));
        BOOST_CHECK(check(variant<none_t, int, std::string> {}));
        BOOST_CHECK(check(variant<none_t, int, std::string> {42}));
        BOOST_CHECK(check(variant<none_t, int, std::string> {std::string {"foo"}}));
    }

}    // namespace

namespace {
    struct stringification_inspector_policy {
        template<class T>
        std::string f(T &x) {
            std::string str;
            detail::stringification_inspector fun {str};
            fun(x);
            return str;
        }

        // only check for compilation for complex types
        template<class T>
        typename std::enable_if<!std::is_integral<T>::value, bool>::type operator()(T &x) {
            BOOST_TEST_MESSAGE("f(x) = " << f(x));
            return true;
        }

        // check result for integral types
        template<class T>
        typename std::enable_if<std::is_integral<T>::value, bool>::type operator()(T &x) {
            BOOST_CHECK_EQUAL(f(x), std::to_string(x));
            return true;
        }

        // check result for bool
        bool operator()(bool &x) {
            BOOST_CHECK_EQUAL(f(x), std::string {x ? "true" : "false"});
            return true;
        }

        // check result for atoms
        bool operator()(atom_value &x) {
            BOOST_CHECK_EQUAL(f(x), "'" + to_string(x) + "'");
            return true;
        }
    };
}    // namespace

BOOST_AUTO_TEST_CASE(stringification_inspector_test) {
    stringification_inspector_policy p;
    test_impl(p);
}

namespace {
    struct binary_serialization_policy {
        execution_unit &context;

        template<class T>
        bool operator()(T &x) {
            std::vector<char> buf;
            binary_serializer f {&context, buf};
            f(x);
            binary_deserializer g {&context, buf};
            T y;
            g(y);
            BOOST_CHECK(x == y);
            return detail::safe_equal(x, y);
        }
    };
}    // namespace

BOOST_AUTO_TEST_CASE(binary_serialization_inspectors_test) {
    actor_system_config cfg;
    cfg.add_message_type<dummy_struct>("dummy_struct");
    actor_system sys {cfg};
    scoped_execution_unit context;
    binary_serialization_policy p {context};
    test_impl(p);
}
