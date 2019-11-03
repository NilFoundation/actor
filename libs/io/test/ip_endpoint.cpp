#define BOOST_TEST_MODULE io_ip_endpoint_test

#include <boost/test/unit_test.hpp>

#include <vector>

#include <nil/mtl/config.hpp>
#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/actor_system_config.hpp>
#include <nil/mtl/serialization/binary_serializer.hpp>
#include <nil/mtl/serialization/binary_deserializer.hpp>

#include <nil/mtl/io/middleman.hpp>
#include <nil/mtl/io/network/interfaces.hpp>
#include <nil/mtl/io/network/ip_endpoint.hpp>

using namespace nil::mtl;
using namespace nil::mtl::io;

namespace {

    class config : public actor_system_config {
    public:
        config() {
            // this will call WSAStartup for network initialization on Windows
            load<io::middleman>();
        }
    };

    struct fixture {

        template<class T, class... Ts>
        std::vector<char> serialize(T &x, Ts &... xs) {
            std::vector<char> buf;
            binary_serializer bs {&context, buf};
            bs(x, xs...);
            return buf;
        }

        template<class T, class... Ts>
        void deserialize(const std::vector<char> &buf, T &x, Ts &... xs) {
            binary_deserializer bd {&context, buf};
            bd(x, xs...);
        }

        fixture() : cfg(), system(cfg), context(&system) {
        }

        config cfg;
        actor_system system;
        scoped_execution_unit context;
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
    std::vector<char> buf = serialize(ep);
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
