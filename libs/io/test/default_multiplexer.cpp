#include <nil/actor/config.hpp>

#define BOOST_TEST_MODULE io_default_multiplexer_test

#include <nil/actor/test/io_dsl.hpp>

#include <vector>
#include <algorithm>

#include <nil/actor/all.hpp>
#include <nil/actor/io/all.hpp>
#include <nil/actor/io/network/default_multiplexer.hpp>
#include <nil/actor/io/network/operation.hpp>

using namespace nil::actor;

namespace {

    struct sub_fixture : test_coordinator_fixture<> {
        io::network::default_multiplexer mpx;

        sub_fixture() : mpx(&sys) {
            // nop
        }

        bool exec_all() {
            size_t count = 0;
            while (mpx.poll_once(false)) {
                ++count;
            }
            return count != 0;
        }
    };

    struct fixture {
        sub_fixture client;

        sub_fixture server;

        void exec_all() {
            while (client.exec_all() || server.exec_all()) {
                // Rince and repeat.
            }
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(default_multiplexer_tests, fixture)

BOOST_AUTO_TEST_CASE(doorman_io_failure_test) {
    BOOST_TEST_MESSAGE("add doorman to server");
    // The multiplexer adds a pipe reader on startup.
    BOOST_CHECK_EQUAL(server.mpx.num_socket_handlers(), 1u);
    auto doorman = unbox(server.mpx.new_tcp_doorman(0, nullptr, false));
    doorman->add_to_loop();

    server.mpx.handle_internal_events();

    BOOST_CHECK_EQUAL(server.mpx.num_socket_handlers(), 2u);
    BOOST_TEST_MESSAGE("trigger I/O failure in doorman");
    doorman->io_failure(&server.mpx, io::network::operation::propagate_error);
    server.mpx.handle_internal_events();

    BOOST_CHECK_EQUAL(server.mpx.num_socket_handlers(), 1u);
}

BOOST_AUTO_TEST_CASE(scribe_io_failure_test) {
    BOOST_TEST_MESSAGE("add doorman to server");
    BOOST_CHECK_EQUAL(server.mpx.num_socket_handlers(), 1u);
    auto doorman = unbox(server.mpx.new_tcp_doorman(0, nullptr, false));
    doorman->add_to_loop();

    server.mpx.handle_internal_events();

    BOOST_CHECK_EQUAL(server.mpx.num_socket_handlers(), 2u);
    BOOST_TEST_MESSAGE("connect to server (add scribe to client)");
    auto scribe = unbox(client.mpx.new_tcp_scribe("localhost", doorman->port()));
    BOOST_CHECK_EQUAL(client.mpx.num_socket_handlers(), 1u);
    scribe->add_to_loop();

    client.mpx.handle_internal_events();

    BOOST_CHECK_EQUAL(client.mpx.num_socket_handlers(), 2u);
    BOOST_TEST_MESSAGE("trigger I/O failure in scribe");
    scribe->io_failure(&client.mpx, io::network::operation::propagate_error);
    client.mpx.handle_internal_events();

    BOOST_CHECK_EQUAL(client.mpx.num_socket_handlers(), 1u);
    BOOST_TEST_MESSAGE("trigger I/O failure in doorman");
    doorman->io_failure(&server.mpx, io::network::operation::propagate_error);
    server.mpx.handle_internal_events();

    BOOST_CHECK_EQUAL(server.mpx.num_socket_handlers(), 1u);
}

BOOST_AUTO_TEST_SUITE_END()
