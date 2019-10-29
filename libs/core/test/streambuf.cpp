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

#define BOOST_TEST_MODULE streambuf_test

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <nil/mtl/config.hpp>
#include <nil/mtl/streambuf.hpp>

using namespace nil::mtl;

BOOST_AUTO_TEST_CASE(signed_arraybuf_test) {
    auto data = std::string {"The quick brown fox jumps over the lazy dog"};
    arraybuf<char> ab {data};
    // Let's read some.
    BOOST_CHECK_EQUAL(static_cast<size_t>(ab.in_avail()), data.size());
    BOOST_CHECK_EQUAL(ab.sgetc(), 'T');
    std::string buf;
    buf.resize(3);
    auto got = ab.sgetn(&buf[0], 3);
    BOOST_CHECK_EQUAL(got, 3);
    BOOST_CHECK_EQUAL(buf, "The");
    BOOST_CHECK_EQUAL(ab.sgetc(), ' ');
    // Exhaust the stream.
    buf.resize(data.size());
    got = ab.sgetn(&buf[0] + 3, static_cast<std::streamsize>(data.size() - 3));
    BOOST_CHECK_EQUAL(static_cast<size_t>(got), data.size() - 3);
    BOOST_CHECK_EQUAL(data, buf);
    BOOST_CHECK_EQUAL(ab.in_avail(), 0);
    // No more.
    auto c = ab.sgetc();
    BOOST_CHECK_EQUAL(c, charbuf::traits_type::eof());
    // Reset the stream and write into it.
    ab.pubsetbuf(&data[0], static_cast<std::streamsize>(data.size()));
    BOOST_CHECK_EQUAL(static_cast<size_t>(ab.in_avail()), data.size());
    auto put = ab.sputn("One", 3);
    BOOST_CHECK_EQUAL(put, 3);
    BOOST_CHECK(data.compare(0, 3, "One") == 0);
}

BOOST_AUTO_TEST_CASE(unsigned_arraybuf_test) {
    using buf_type = arraybuf<uint8_t>;
    std::vector<uint8_t> data = {0x0a, 0x0b, 0x0c, 0x0d};
    buf_type ab {data};
    decltype(data) buf;
    std::copy(std::istreambuf_iterator<uint8_t> {&ab}, std::istreambuf_iterator<uint8_t> {}, std::back_inserter(buf));
    BOOST_CHECK(data == buf);
    // Relative positioning.
    using pos_type = buf_type::pos_type;
    using int_type = buf_type::int_type;
    BOOST_CHECK_EQUAL(ab.pubseekoff(2, std::ios::beg, std::ios::in), pos_type {2});
    BOOST_CHECK_EQUAL(ab.sbumpc(), int_type {0x0c});
    BOOST_CHECK_EQUAL(ab.sgetc(), int_type {0x0d});
    BOOST_CHECK_EQUAL(ab.pubseekoff(0, std::ios::cur, std::ios::in), pos_type {3});
    BOOST_CHECK_EQUAL(ab.pubseekoff(-2, std::ios::cur, std::ios::in), pos_type {1});
    BOOST_CHECK_EQUAL(ab.sgetc(), int_type {0x0b});
    BOOST_CHECK_EQUAL(ab.pubseekoff(-4, std::ios::end, std::ios::in), pos_type {0});
    BOOST_CHECK_EQUAL(ab.sgetc(), int_type {0x0a});
    // Absolute positioning.
    BOOST_CHECK_EQUAL(ab.pubseekpos(1, std::ios::in), pos_type {1});
    BOOST_CHECK_EQUAL(ab.sgetc(), int_type {0x0b});
    BOOST_CHECK_EQUAL(ab.pubseekpos(3, std::ios::in), pos_type {3});
    BOOST_CHECK_EQUAL(ab.sbumpc(), int_type {0x0d});
    BOOST_CHECK_EQUAL(ab.in_avail(), std::streamsize {0});
}

BOOST_AUTO_TEST_CASE(containerbuf_test) {
    std::string data {
        "Habe nun, ach! Philosophie,\n"
        "Juristerei und Medizin,\n"
        "Und leider auch Theologie\n"
        "Durchaus studiert, mit heißem Bemühn.\n"
        "Da steh ich nun, ich armer Tor!\n"
        "Und bin so klug als wie zuvor"};
    // Write some data.
    std::vector<char> buf;
    vectorbuf vb {buf};
    auto put = vb.sputn(data.data(), static_cast<std::streamsize>(data.size()));
    BOOST_CHECK_EQUAL(static_cast<size_t>(put), data.size());
    put = vb.sputn(";", 1);
    BOOST_CHECK_EQUAL(put, 1);
    std::string target;
    std::copy(buf.begin(), buf.end(), std::back_inserter(target));
    BOOST_CHECK_EQUAL(data + ';', target);
    // Check "overflow" on a new stream.
    buf.clear();
    vectorbuf vb2 {buf};
    auto chr = vb2.sputc('x');
    BOOST_CHECK_EQUAL(chr, 'x');
    // Let's read some data into a stream.
    buf.clear();
    containerbuf<std::string> scb {data};
    std::copy(std::istreambuf_iterator<char> {&scb}, std::istreambuf_iterator<char> {}, std::back_inserter(buf));
    BOOST_CHECK_EQUAL(buf.size(), data.size());
    BOOST_CHECK(std::equal(buf.begin(), buf.end(), data.begin() /*, data.end() */));
    // We're done, nothing to see here, please move along.
    BOOST_CHECK_EQUAL(scb.sgetc(), containerbuf<std::string>::traits_type::eof());
    // Let's read again, but now in one big block.
    buf.clear();
    containerbuf<std::string> sib2 {data};
    buf.resize(data.size());
    auto got = sib2.sgetn(&buf[0], static_cast<std::streamsize>(buf.size()));
    BOOST_CHECK_EQUAL(static_cast<size_t>(got), data.size());
    BOOST_CHECK_EQUAL(buf.size(), data.size());
    BOOST_CHECK(std::equal(buf.begin(), buf.end(), data.begin() /*, data.end() */));
}

BOOST_AUTO_TEST_CASE(containerbuf_reset_get_area_test) {
    std::string str {"foobar"};
    std::vector<char> buf;
    vectorbuf vb {buf};
    // We can always write to the underlying buffer; no put area needed.
    auto n = vb.sputn(str.data(), str.size());
    BOOST_REQUIRE_EQUAL(n, 6);
    // Readjust the get area.
    BOOST_REQUIRE_EQUAL(buf.size(), 6u);
    vb.pubsetbuf(buf.data() + 3, buf.size() - 3);
    // Now read from a new get area into a buffer.
    char bar[3];
    n = vb.sgetn(bar, 3);
    BOOST_CHECK_EQUAL(n, 3);
    BOOST_CHECK_EQUAL(std::string(bar, 3), "bar");
    // Synchronize the get area after having messed with the underlying buffer.
    buf.resize(1);
    BOOST_CHECK_EQUAL(vb.pubsync(), 0);
    BOOST_CHECK_EQUAL(vb.sbumpc(), 'f');
    BOOST_CHECK_EQUAL(vb.in_avail(), 0);
}
