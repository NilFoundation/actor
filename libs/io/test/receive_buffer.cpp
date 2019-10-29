#define BOOST_TEST_MODULE io_receive_buffer_test

#include <boost/test/unit_test.hpp>

#include <algorithm>

#include <nil/mtl/config.hpp>
#include <nil/mtl/io/network/receive_buffer.hpp>

using namespace nil::mtl;
using nil::mtl::io::network::receive_buffer;

namespace {

    struct fixture {
        receive_buffer a;
        receive_buffer b;
        std::vector<char> vec;

        fixture() : b(1024ul), vec {'h', 'a', 'l', 'l', 'o'} {
            // nop
        }

        std::string as_string(const receive_buffer &xs) {
            std::string result;
            for (auto &x : xs) {
                result += static_cast<char>(x);
            }
            return result;
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(receive_buffer_tests, fixture)

BOOST_AUTO_TEST_CASE(constuctors_test) {
    BOOST_CHECK_EQUAL(a.size(), 0ul);
    BOOST_CHECK_EQUAL(a.capacity(), 0ul);
    BOOST_CHECK_EQUAL(a.data(), nullptr);
    BOOST_CHECK(a.empty());
    BOOST_CHECK_EQUAL(b.size(), 1024ul);
    BOOST_CHECK_EQUAL(b.capacity(), 1024ul);
    BOOST_CHECK(b.data() != nullptr);
    BOOST_CHECK(!b.empty());
    receive_buffer other {std::move(b)};
    BOOST_CHECK_EQUAL(other.size(), 1024ul);
    BOOST_CHECK_EQUAL(other.capacity(), 1024ul);
    BOOST_CHECK(other.data() != nullptr);
    BOOST_CHECK(!other.empty());
}

BOOST_AUTO_TEST_CASE(reserve_test) {
    a.reserve(0);
    BOOST_CHECK_EQUAL(a.size(), 0ul);
    BOOST_CHECK_EQUAL(a.capacity(), 0ul);
    BOOST_CHECK(a.data() == nullptr);
    BOOST_CHECK(a.empty());
    a.reserve(1024);
    BOOST_CHECK_EQUAL(a.size(), 0ul);
    BOOST_CHECK_EQUAL(a.capacity(), 1024ul);
    BOOST_CHECK(a.data() != nullptr);
    BOOST_CHECK_EQUAL(a.begin(), a.end());
    BOOST_CHECK(a.empty());
    a.reserve(512);
    BOOST_CHECK_EQUAL(a.size(), 0ul);
    BOOST_CHECK_EQUAL(a.capacity(), 1024ul);
    BOOST_CHECK(a.data() != nullptr);
    BOOST_CHECK_EQUAL(a.begin(), a.end());
    BOOST_CHECK(a.empty());
}

BOOST_AUTO_TEST_CASE(resize_test) {
    a.resize(512);
    BOOST_CHECK_EQUAL(a.size(), 512ul);
    BOOST_CHECK_EQUAL(a.capacity(), 512ul);
    BOOST_CHECK(a.data() != nullptr);
    BOOST_CHECK(!a.empty());
    b.resize(512);
    BOOST_CHECK_EQUAL(b.size(), 512ul);
    BOOST_CHECK_EQUAL(b.capacity(), 1024ul);
    BOOST_CHECK(b.data() != nullptr);
    BOOST_CHECK(!b.empty());
    a.resize(1024);
    std::fill(a.begin(), a.end(), 'a');
    auto cnt = 0;
    BOOST_CHECK(std::all_of(a.begin(), a.end(), [&](char c) {
        ++cnt;
        return c == 'a';
    }));
    BOOST_CHECK_EQUAL(cnt, 1024);
    a.resize(10);
    cnt = 0;
    BOOST_CHECK(std::all_of(a.begin(), a.end(), [&](char c) {
        ++cnt;
        return c == 'a';
    }));
    BOOST_CHECK_EQUAL(cnt, 10);
    a.resize(1024);
    cnt = 0;
    BOOST_CHECK(std::all_of(a.begin(), a.end(), [&](char c) {
        ++cnt;
        return c == 'a';
    }));
    BOOST_CHECK_EQUAL(cnt, 1024);
}

BOOST_AUTO_TEST_CASE(push_back_test) {
    for (auto c : vec) {
        a.push_back(c);
    }
    BOOST_CHECK_EQUAL(vec.size(), a.size());
    BOOST_CHECK_EQUAL(a.capacity(), 8ul);
    BOOST_CHECK(a.data() != nullptr);
    BOOST_CHECK(!a.empty());
    BOOST_CHECK(std::equal(vec.begin(), vec.end(), a.begin()));
}

BOOST_AUTO_TEST_CASE(insert_test) {
    for (auto c : vec) {
        a.insert(a.end(), c);
    }
    BOOST_CHECK_EQUAL(as_string(a), "hallo");
    a.insert(a.begin(), '!');
    BOOST_CHECK_EQUAL(as_string(a), "!hallo");
    a.insert(a.begin() + 4, '-');
    BOOST_CHECK_EQUAL(as_string(a), "!hal-lo");
    std::string foo = "foo:";
    a.insert(a.begin() + 1, foo.begin(), foo.end());
    BOOST_CHECK_EQUAL(as_string(a), "!foo:hal-lo");
    std::string bar = ":bar";
    a.insert(a.end(), bar.begin(), bar.end());
    BOOST_CHECK_EQUAL(as_string(a), "!foo:hal-lo:bar");
}

BOOST_AUTO_TEST_CASE(shrink_to_fit_test) {
    a.shrink_to_fit();
    BOOST_CHECK_EQUAL(a.size(), 0ul);
    BOOST_CHECK_EQUAL(a.capacity(), 0ul);
    BOOST_CHECK(a.data() == nullptr);
    BOOST_CHECK(a.empty());
}

BOOST_AUTO_TEST_CASE(swap_test) {
    for (auto c : vec) {
        a.push_back(c);
    }
    std::swap(a, b);
    BOOST_CHECK_EQUAL(a.size(), 1024ul);
    BOOST_CHECK_EQUAL(a.capacity(), 1024ul);
    BOOST_CHECK(a.data() != nullptr);
    BOOST_CHECK_EQUAL(b.size(), vec.size());
    BOOST_CHECK_EQUAL(std::distance(b.begin(), b.end()), static_cast<receive_buffer::difference_type>(vec.size()));
    BOOST_CHECK_EQUAL(b.capacity(), 8ul);
    BOOST_CHECK(b.data() != nullptr);
    BOOST_CHECK_EQUAL(*(b.data() + 0), 'h');
    BOOST_CHECK_EQUAL(*(b.data() + 1), 'a');
    BOOST_CHECK_EQUAL(*(b.data() + 2), 'l');
    BOOST_CHECK_EQUAL(*(b.data() + 3), 'l');
    BOOST_CHECK_EQUAL(*(b.data() + 4), 'o');
}

BOOST_AUTO_TEST_SUITE_END()
