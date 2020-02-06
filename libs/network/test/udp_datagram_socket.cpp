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

#define BOOST_TEST_MODULE udp_datagram_socket

#include <nil/mtl/network/udp_datagram_socket.hpp>

#include <nil/mtl/test/host_fixture.hpp>
#include <nil/mtl/test/dsl.hpp>

#include <nil/mtl/serialization/binary_serializer.hpp>
#include <nil/mtl/detail/net_syscall.hpp>
#include <nil/mtl/detail/socket_sys_includes.hpp>
#include <nil/mtl/ip_address.hpp>
#include <nil/mtl/ip_endpoint.hpp>
#include <nil/mtl/ipv4_address.hpp>
#include <nil/mtl/network/ip.hpp>

using namespace nil::mtl;
using namespace nil::mtl::network;
using namespace nil::mtl::network::ip;

namespace {

    constexpr string_view hello_test = "Hello test!";

    struct fixture : host_fixture {
        fixture() : host_fixture(), buf(1024) {
            addresses = local_addresses("localhost");
            BOOST_CHECK(!addresses.empty());
            ep = ip_endpoint(*addresses.begin(), 0);
            auto send_pair = unbox(make_udp_datagram_socket(ep));
            send_socket = send_pair.first;
            auto receive_pair = unbox(make_udp_datagram_socket(ep));
            receive_socket = receive_pair.first;
            ep.port(ntohs(receive_pair.second));
        }

        ~fixture() {
            close(send_socket);
            close(receive_socket);
        }

        std::vector<ip_address> addresses;
        spawner_config cfg;
        spawner sys {cfg};
        ip_endpoint ep;
        udp_datagram_socket send_socket;
        udp_datagram_socket receive_socket;
        std::vector<byte> buf;
    };

    error read_from_socket(udp_datagram_socket sock, std::vector<byte> &buf) {
        uint8_t receive_attempts = 0;
        variant<std::pair<size_t, ip_endpoint>, sec> read_ret;
        do {
            read_ret = read(sock, buf);
            if (auto read_res = get_if<std::pair<size_t, ip_endpoint>>(&read_ret)) {
                buf.resize(read_res->first);
            } else if (get<sec>(read_ret) != sec::unavailable_or_would_block) {
                return make_error(get<sec>(read_ret), "read failed");
            }
            if (++receive_attempts > 100)
                return make_error(sec::runtime_error, "too many unavailable_or_would_blocks");
        } while (read_ret.index() != 0);
        return none;
    }

    struct header {
        header(size_t payload_size) : payload_size(payload_size) {
            // nop
        }

        header() : header(0) {
            // nop
        }

        template<class Inspector>
        friend typename Inspector::result_type inspect(Inspector &f, header &x) {
            return f(meta::type_name("header"), x.payload_size);
        }

        size_t payload_size;
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(udp_datagram_socket_test, fixture)

BOOST_AUTO_TEST_CASE(read / write using span<byte>) {
    if (auto err = nonblocking(socket_cast<net::socket>(receive_socket), true))
        BOOST_FAIL("setting socket to nonblocking failed: " << err);
    BOOST_CHECK_EQUAL(read(receive_socket, buf), sec::unavailable_or_would_block);
    BOOST_TEST_MESSAGE("sending data to " << to_string(ep));
    BOOST_CHECK_EQUAL(write(send_socket, as_bytes(make_span(hello_test)), ep), hello_test.size());
    BOOST_CHECK_EQUAL(read_from_socket(receive_socket, buf), none);
    string_view received {reinterpret_cast<const char *>(buf.data()), buf.size()};
    BOOST_CHECK_EQUAL(received, hello_test);
}

BOOST_AUTO_TEST_CASE(read / write using span<std::vector<byte> *>) {
    // generate header and payload in separate buffers
    header hdr {hello_test.size()};
    std::vector<byte> hdr_buf;
    binary_serializer sink(sys, hdr_buf);
    if (auto err = sink(hdr))
        BOOST_FAIL("serializing payload failed" << sys.render(err));
    auto bytes = as_bytes(make_span(hello_test));
    std::vector<byte> payload_buf(bytes.begin(), bytes.end());
    auto packet_size = hdr_buf.size() + payload_buf.size();
    std::vector<std::vector<byte> *> bufs {&hdr_buf, &payload_buf};
    BOOST_CHECK_EQUAL(write(send_socket, bufs, ep), packet_size);
    // receive both as one single packet.
    buf.resize(packet_size);
    BOOST_CHECK_EQUAL(read_from_socket(receive_socket, buf), none);
    BOOST_CHECK_EQUAL(buf.size(), packet_size);
    binary_deserializer source(nullptr, buf);
    header recv_hdr;
    if (auto err = source(recv_hdr))
        BOOST_FAIL("serializing failed: " << err);
    BOOST_CHECK_EQUAL(hdr.payload_size, recv_hdr.payload_size);
    string_view received {reinterpret_cast<const char *>(buf.data()) + sizeof(header), buf.size() - sizeof(header)};
    BOOST_CHECK_EQUAL(received, hello_test);
}

BOOST_AUTO_TEST_SUITE_END()
