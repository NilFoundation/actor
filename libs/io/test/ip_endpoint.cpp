#define BOOST_TEST_MODULE io_ip_endpoint_test

#include <boost/test/unit_test.hpp>

#include <vector>

#include <nil/actor/test/dsl.hpp>
#include <nil/actor/config.hpp>
#include <nil/actor/spawner.hpp>
#include <nil/actor/spawner_config.hpp>
#include <nil/actor/binary_serializer.hpp>
#include <nil/actor/binary_deserializer.hpp>

#include <nil/actor/io/middleman.hpp>
#include <nil/actor/io/network/interfaces.hpp>
#include <nil/actor/io/network/ip_endpoint.hpp>

using namespace nil::actor;
using namespace nil::actor::io;

namespace {

    class config : public spawner_config {
    public:
        config() {
            // this will call WSAStartup for network initialization on Windows
            load<io::middleman>();
        }
    };

    struct fixture : test_coordinator_fixture<> {
        template<class T, class... Ts>
        auto serialize(T &x, Ts &... xs) {
            byte_buffer buf;
            binary_serializer sink {sys, buf};
            if (auto err = sink(x, xs...))
                BOOST_FAIL("serialization failed: " << sys.render(err));
            return buf;
        }

        template<class Buffer, class T, class... Ts>
        void deserialize(const Buffer &buf, T &x, Ts &... xs) {
            binary_deserializer source {sys, buf};
            if (auto err = source(x, xs...))
                BOOST_FAIL("serialization failed: " << sys.render(err));
        }
    };
}    // namespace

BOOST_FIXTURE_TEST_SUITE(ep_endpoint_tests, fixture)

BOOST_AUTO_TEST_CASE(ip_endpoint_test) {
    // create an empty endpoint
    network::ip_endpoint ep;
    ep.clear();
    BOOST_CHECK_EQUAL("", network::host(ep));
    BOOST_CHECK_EQUAL(uint16_t {0}, network::port(ep));
    BOOST_CHECK_EQUAL(size_t {0}, *ep.length());
    // fill it with data from a local endpoint
    network::interfaces::get_endpoint("localhost", 12345, ep);
    // save the data
    auto h = network::host(ep);
    auto p = network::port(ep);
    auto l = *ep.length();
    BOOST_CHECK("localhost" == h || "127.0.0.1" == h || "::1" == h);
    BOOST_CHECK_EQUAL(12345, p);
    BOOST_CHECK(0 < l);
    // serialize the endpoint and clear it
    byte_buffer buf = serialize(ep);
    auto save = ep;
    ep.clear();
    BOOST_CHECK_EQUAL("", network::host(ep));
    BOOST_CHECK_EQUAL(uint16_t {0}, network::port(ep));
    BOOST_CHECK_EQUAL(size_t {0}, *ep.length());
    // deserialize the data and check if it was load successfully
    deserialize(buf, ep);
    BOOST_CHECK_EQUAL(h, network::host(ep));
    BOOST_CHECK_EQUAL(uint16_t {p}, network::port(ep));
    BOOST_CHECK_EQUAL(size_t {l}, *ep.length());
    BOOST_CHECK(save == ep);
}

BOOST_AUTO_TEST_SUITE_END()
