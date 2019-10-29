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

#define BOOST_TEST_MODULE uri_test

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <nil/mtl/config.hpp>

#include <nil/mtl/test/dsl.hpp>

#include <nil/mtl/ipv4_address.hpp>
#include <nil/mtl/uri.hpp>
#include <nil/mtl/uri_builder.hpp>

using namespace nil::mtl;

namespace {

    struct authority_separator_t {
    } authority_separator;

    struct path_separator_t {
    } path_separator;

    struct uri_str_builder {
        std::string res;

        uri_str_builder() : res("http:") {
            // nop
        }

        uri_str_builder &add() {
            return *this;
        }

        template<class T, class... Ts>
        uri_str_builder &add(const T &x, const Ts &... xs) {
            res += x;
            return add(xs...);
        }

        template<class... Ts>
        uri_str_builder &add(const authority_separator_t &, const Ts &... xs) {
            if (res.back() == ':') {
                return add("//", xs...);
            }
            return add(xs...);
        }

        template<class... Ts>
        uri_str_builder &add(const path_separator_t &, const Ts &... xs) {
            if (res.back() != ':') {
                return add("/", xs...);
            }
            return add(xs...);
        }

        uri_str_builder &userinfo(std::string str) {
            return add(authority_separator, str, '@');
        }

        uri_str_builder &host(std::string str) {
            return add(authority_separator, str);
        }

        uri_str_builder &host(ip_address addr) {
            return add(authority_separator, '[', to_string(addr), ']');
        }

        uri_str_builder &port(uint16_t value) {
            return add(':', std::to_string(value));
        }

        uri_str_builder &path(std::string str) {
            return add(path_separator, str);
        }

        uri_str_builder &query(uri::query_map map) {
            if (map.empty()) {
                return *this;
            }
            auto print_kvp = [&](const uri::query_map::value_type &kvp) {
                res += kvp.first;
                res += '=';
                res += kvp.second;
            };
            res += '?';
            auto i = map.begin();
            print_kvp(*i);
            for (++i; i != map.end(); ++i) {
                res += '&';
                print_kvp(*i);
            }
            return *this;
        }

        uri_str_builder &fragment(std::string str) {
            return add('#', str);
        }

        std::string operator*() {
            using std::swap;
            std::string str = "http:";
            swap(str, res);
            return str;
        }
    };

    struct fixture {
        // -- member types
        // -----------------------------------------------------------

        using buffer = std::vector<char>;

        // -- constructors, destructors, and assignment operators
        // --------------------

        fixture() {
            http.scheme("http");
        }

        // -- member variables
        // -------------------------------------------------------

        uri_builder http;

        uri_str_builder http_str;

        // -- utility functions
        // ------------------------------------------------------

        buffer serialize(uri x) {
            buffer buf;
            binary_serializer dst {nullptr, buf};
            auto err = inspect(dst, x);
            if (err) {
                BOOST_FAIL("unable to serialize " << to_string(x) << ": " << to_string(err));
            }
            return buf;
        }

        uri deserialize(buffer buf) {
            uri result;
            binary_deserializer src {nullptr, buf};
            auto err = inspect(src, result);
            if (err) {
                BOOST_FAIL("unable to deserialize from buffer: " << to_string(err));
            }
            return result;
        }
    };

    struct me_t {
    } me;

    template<class T>
    T &operator<<(T &builder, me_t) {
        return builder.userinfo("me");
    }

    struct node_t {
    } node;

    template<class T>
    T &operator<<(T &builder, node_t) {
        return builder.host("node");
    }

    struct port80_t {
    } port80;

    template<class T>
    T &operator<<(T &builder, port80_t) {
        return builder.port(80);
    }

    struct file_t {
    } file;

    template<class T>
    T &operator<<(T &builder, file_t) {
        return builder.path("file");
    }

    struct frag_t {
    } frag;

    template<class T>
    T &operator<<(T &builder, frag_t) {
        return builder.fragment("42");
    }

    struct kvp_t {
    } kvp;

    template<class T>
    T &operator<<(T &builder, kvp_t) {
        return builder.query(uri::query_map {{"a", "1"}, {"b", "2"}});
    }

    uri operator*(uri_builder &builder) {
        auto result = builder.make();
        builder = uri_builder();
        auto scheme = result.scheme();
        builder.scheme(std::string {scheme.begin(), scheme.end()});
        return result;
    }

    uri operator"" _u(const char *cstr, size_t cstr_len) {
        uri result;
        string_view str {cstr, cstr_len};
        auto err = parse(str, result);
        if (err) {
            BOOST_FAIL("error while parsing " << str << ": " << to_string(err));
        }
        return result;
    }

    bool operator"" _i(const char *cstr, size_t cstr_len) {
        uri result;
        string_view str {cstr, cstr_len};
        auto err = parse(str, result);
        return err != none;
    }

}    // namespace

BOOST_FIXTURE_TEST_SUITE(uri_tests, fixture)

BOOST_AUTO_TEST_CASE(constructing_test) {
    uri x;
    BOOST_CHECK_EQUAL(x.empty(), true);
    BOOST_CHECK_EQUAL(x.str(), "");
}

#define BUILD(components) BOOST_CHECK(*(http << components) == *(http_str << components))

BOOST_AUTO_TEST_CASE(builder_construction_test) {
    auto minimal = *(http << file);
    BOOST_CHECK_EQUAL(minimal.empty(), false);
    BOOST_CHECK(minimal == "http:file");
    // all combinations of components
    BUILD(file);
    BUILD(file << kvp);
    BUILD(file << frag);
    BUILD(file << kvp << frag);
    BUILD(node);
    BUILD(node << frag);
    BUILD(node << kvp);
    BUILD(node << kvp << frag);
    BUILD(node << port80);
    BUILD(node << port80 << frag);
    BUILD(node << port80 << kvp);
    BUILD(node << port80 << kvp << frag);
    BUILD(me << node);
    BUILD(me << node << kvp);
    BUILD(me << node << frag);
    BUILD(me << node << kvp << frag);
    BUILD(me << node << port80);
    BUILD(me << node << port80 << frag);
    BUILD(me << node << port80 << kvp);
    BUILD(me << node << port80 << kvp << frag);
    BUILD(node << file);
    BUILD(node << file << frag);
    BUILD(node << file << kvp);
    BUILD(node << file << kvp << frag);
    BUILD(node << port80 << file);
    BUILD(node << port80 << file << frag);
    BUILD(node << port80 << file << kvp);
    BUILD(node << port80 << file << kvp << frag);
    BUILD(me << node << file);
    BUILD(me << node << file << frag);
    BUILD(me << node << file << kvp);
    BUILD(me << node << file << kvp << frag);
    BUILD(me << node << port80 << file);
    BUILD(me << node << port80 << file << frag);
    BUILD(me << node << port80 << file << kvp);
    BUILD(me << node << port80 << file << kvp << frag);
    // percent encoding
    auto escaped =
        uri_builder {}.scheme("hi there").userinfo("it's").host("me/").path("file 1").fragment("[42]").make();
    BOOST_CHECK(escaped == "hi%20there://it%27s@me%2F/file%201#%5B42%5D");
}

#define ROUNDTRIP(str) BOOST_CHECK(str##_u == str)

BOOST_AUTO_TEST_CASE(from_string_test) {
    // all combinations of components
    ROUNDTRIP("http:file");
    ROUNDTRIP("http:file?a=1&b=2");
    ROUNDTRIP("http:file#42");
    ROUNDTRIP("http:file?a=1&b=2#42");
    ROUNDTRIP("http://node");
    ROUNDTRIP("http://node?a=1&b=2");
    ROUNDTRIP("http://node#42");
    ROUNDTRIP("http://node?a=1&b=2#42");
    ROUNDTRIP("http://node:80");
    ROUNDTRIP("http://node:80?a=1&b=2");
    ROUNDTRIP("http://node:80#42");
    ROUNDTRIP("http://node:80?a=1&b=2#42");
    ROUNDTRIP("http://me@node");
    ROUNDTRIP("http://me@node?a=1&b=2");
    ROUNDTRIP("http://me@node#42");
    ROUNDTRIP("http://me@node?a=1&b=2#42");
    ROUNDTRIP("http://me@node:80");
    ROUNDTRIP("http://me@node:80?a=1&b=2");
    ROUNDTRIP("http://me@node:80#42");
    ROUNDTRIP("http://me@node:80?a=1&b=2#42");
    ROUNDTRIP("http://node/file");
    ROUNDTRIP("http://node/file?a=1&b=2");
    ROUNDTRIP("http://node/file#42");
    ROUNDTRIP("http://node/file?a=1&b=2#42");
    ROUNDTRIP("http://node:80/file");
    ROUNDTRIP("http://node:80/file?a=1&b=2");
    ROUNDTRIP("http://node:80/file#42");
    ROUNDTRIP("http://node:80/file?a=1&b=2#42");
    ROUNDTRIP("http://me@node/file");
    ROUNDTRIP("http://me@node/file?a=1&b=2");
    ROUNDTRIP("http://me@node/file#42");
    ROUNDTRIP("http://me@node/file?a=1&b=2#42");
    ROUNDTRIP("http://me@node:80/file");
    ROUNDTRIP("http://me@node:80/file?a=1&b=2");
    ROUNDTRIP("http://me@node:80/file#42");
    ROUNDTRIP("http://me@node:80/file?a=1&b=2#42");
    // all combinations of with IPv6 host
    ROUNDTRIP("http://[::1]");
    ROUNDTRIP("http://[::1]?a=1&b=2");
    ROUNDTRIP("http://[::1]#42");
    ROUNDTRIP("http://[::1]?a=1&b=2#42");
    ROUNDTRIP("http://[::1]:80");
    ROUNDTRIP("http://[::1]:80?a=1&b=2");
    ROUNDTRIP("http://[::1]:80#42");
    ROUNDTRIP("http://[::1]:80?a=1&b=2#42");
    ROUNDTRIP("http://me@[::1]");
    ROUNDTRIP("http://me@[::1]?a=1&b=2");
    ROUNDTRIP("http://me@[::1]#42");
    ROUNDTRIP("http://me@[::1]?a=1&b=2#42");
    ROUNDTRIP("http://me@[::1]:80");
    ROUNDTRIP("http://me@[::1]:80?a=1&b=2");
    ROUNDTRIP("http://me@[::1]:80#42");
    ROUNDTRIP("http://me@[::1]:80?a=1&b=2#42");
    ROUNDTRIP("http://[::1]/file");
    ROUNDTRIP("http://[::1]/file?a=1&b=2");
    ROUNDTRIP("http://[::1]/file#42");
    ROUNDTRIP("http://[::1]/file?a=1&b=2#42");
    ROUNDTRIP("http://[::1]:80/file");
    ROUNDTRIP("http://[::1]:80/file?a=1&b=2");
    ROUNDTRIP("http://[::1]:80/file#42");
    ROUNDTRIP("http://[::1]:80/file?a=1&b=2#42");
    ROUNDTRIP("http://me@[::1]/file");
    ROUNDTRIP("http://me@[::1]/file?a=1&b=2");
    ROUNDTRIP("http://me@[::1]/file#42");
    ROUNDTRIP("http://me@[::1]/file?a=1&b=2#42");
    ROUNDTRIP("http://me@[::1]:80/file");
    ROUNDTRIP("http://me@[::1]:80/file?a=1&b=2");
    ROUNDTRIP("http://me@[::1]:80/file#42");
    ROUNDTRIP("http://me@[::1]:80/file?a=1&b=2#42");
    // percent encoding
    ROUNDTRIP("hi%20there://it%27s@me%21/file%201#%5B42%5D");
}

#undef ROUNDTRIP

BOOST_AUTO_TEST_CASE(empty_components_test) {
    BOOST_CHECK("foo:/"_u == "foo:/");
    BOOST_CHECK("foo:/#"_u == "foo:/");
    BOOST_CHECK("foo:/?"_u == "foo:/");
    BOOST_CHECK("foo:/?#"_u == "foo:/");
    BOOST_CHECK("foo:bar#"_u == "foo:bar");
    BOOST_CHECK("foo:bar?"_u == "foo:bar");
    BOOST_CHECK("foo:bar?#"_u == "foo:bar");
    BOOST_CHECK("foo://bar#"_u == "foo://bar");
    BOOST_CHECK("foo://bar?"_u == "foo://bar");
    BOOST_CHECK("foo://bar?#"_u == "foo://bar");
}

BOOST_AUTO_TEST_CASE(invalid_uris_test) {
    BOOST_CHECK("http"_i);
    BOOST_CHECK("http://"_i);
    BOOST_CHECK("http://foo:66000"_i);
}

#define SERIALIZATION_ROUNDTRIP(str) BOOST_CHECK(deserialize(serialize(str##_u)) == str)

BOOST_AUTO_TEST_CASE(serialization_test) {
    // all combinations of components
    SERIALIZATION_ROUNDTRIP("http:file");
    SERIALIZATION_ROUNDTRIP("http:file?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http:file#42");
    SERIALIZATION_ROUNDTRIP("http:file?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://node");
    SERIALIZATION_ROUNDTRIP("http://node?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://node#42");
    SERIALIZATION_ROUNDTRIP("http://node?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://node:80");
    SERIALIZATION_ROUNDTRIP("http://node:80?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://node:80#42");
    SERIALIZATION_ROUNDTRIP("http://node:80?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://me@node");
    SERIALIZATION_ROUNDTRIP("http://me@node?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://me@node#42");
    SERIALIZATION_ROUNDTRIP("http://me@node?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://me@node:80");
    SERIALIZATION_ROUNDTRIP("http://me@node:80?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://me@node:80#42");
    SERIALIZATION_ROUNDTRIP("http://me@node:80?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://node/file");
    SERIALIZATION_ROUNDTRIP("http://node/file?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://node/file#42");
    SERIALIZATION_ROUNDTRIP("http://node/file?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://node:80/file");
    SERIALIZATION_ROUNDTRIP("http://node:80/file?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://node:80/file#42");
    SERIALIZATION_ROUNDTRIP("http://node:80/file?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://me@node/file");
    SERIALIZATION_ROUNDTRIP("http://me@node/file?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://me@node/file#42");
    SERIALIZATION_ROUNDTRIP("http://me@node/file?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://me@node:80/file");
    SERIALIZATION_ROUNDTRIP("http://me@node:80/file?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://me@node:80/file#42");
    SERIALIZATION_ROUNDTRIP("http://me@node:80/file?a=1&b=2#42");
    // all combinations of with IPv6 host
    SERIALIZATION_ROUNDTRIP("http://[::1]");
    SERIALIZATION_ROUNDTRIP("http://[::1]?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://[::1]#42");
    SERIALIZATION_ROUNDTRIP("http://[::1]?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://[::1]:80");
    SERIALIZATION_ROUNDTRIP("http://[::1]:80?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://[::1]:80#42");
    SERIALIZATION_ROUNDTRIP("http://[::1]:80?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]#42");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]:80");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]:80?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]:80#42");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]:80?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://[::1]/file");
    SERIALIZATION_ROUNDTRIP("http://[::1]/file?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://[::1]/file#42");
    SERIALIZATION_ROUNDTRIP("http://[::1]/file?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://[::1]:80/file");
    SERIALIZATION_ROUNDTRIP("http://[::1]:80/file?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://[::1]:80/file#42");
    SERIALIZATION_ROUNDTRIP("http://[::1]:80/file?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]/file");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]/file?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]/file#42");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]/file?a=1&b=2#42");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]:80/file");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]:80/file?a=1&b=2");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]:80/file#42");
    SERIALIZATION_ROUNDTRIP("http://me@[::1]:80/file?a=1&b=2#42");
    // percent encoding
    SERIALIZATION_ROUNDTRIP("hi%20there://it%27s@me%21/file%201#%5B42%5D");
}

#undef SERIALIZATION_ROUNDTRIP

BOOST_AUTO_TEST_SUITE_END()
