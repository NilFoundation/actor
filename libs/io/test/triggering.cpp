#define BOOST_TEST_MODULE io_triggering_test

#include <boost/test/unit_test.hpp>

#include <memory>
#include <iostream>

#include <nil/actor/config.hpp>
#include <nil/actor/all.hpp>
#include <nil/actor/io/all.hpp>

using namespace std;
using namespace nil::actor;
using namespace nil::actor::io;

namespace {

    // -- client implementation, used for both test servers ------------------------

    behavior client(broker *self, connection_handle hdl) {
        std::vector<char> buf;
        buf.resize(200);
        std::iota(buf.begin(), buf.end(), 0);
        self->write(hdl, buf.size(), buf.data());
        BOOST_REQUIRE_EQUAL(self->wr_buf(hdl).size(), 200u);
        self->configure_read(hdl, receive_policy::at_least(1));
        self->flush(hdl);
        return {[=](const new_data_msg &) { BOOST_FAIL("server unexpectedly sent data"); },
                [=](const connection_closed_msg &) { self->quit(); }};
    }

    // -- first test server --------------------------------------------------------

    struct server1_state {
        size_t received = 0;
        connection_handle peer = invalid_connection_handle;
    };

    // consumes 5 more tokens, then waits for passivated message to shutdown
    behavior server1_stage4(stateful_actor<server1_state, broker> *self) {
        BOOST_TEST_MESSAGE("enter server stage 4");
        self->trigger(self->state.peer, 5);
        return {[=](const new_data_msg &dm) {
                    BOOST_REQUIRE_EQUAL(dm.buf.size(), 10u);
                    self->state.received += 1;
                },
                [=](const connection_passivated_msg &cp) {
                    BOOST_REQUIRE(cp.handle == self->state.peer);
                    BOOST_REQUIRE_EQUAL(self->state.received, 15u);
                    BOOST_REQUIRE(self->state.peer != invalid_connection_handle);
                    // delay new tokens to force MM to remove this broker from its loop
                    BOOST_TEST_MESSAGE("server is done");
                    self->quit();
                }};
    }

    // consumes 5 more tokens, then waits for passivated message to send itself
    // a message that generates 5 more (force MM to actually remove this broker
    // from its event loop and then re-adding it)
    behavior server1_stage3(stateful_actor<server1_state, broker> *self) {
        BOOST_TEST_MESSAGE("enter server stage 3");
        self->trigger(self->state.peer, 5);
        return {[=](const new_data_msg &dm) {
                    BOOST_REQUIRE_EQUAL(dm.buf.size(), 10u);
                    self->state.received += 1;
                },
                [=](const connection_passivated_msg &cp) {
                    BOOST_REQUIRE(cp.handle == self->state.peer);
                    BOOST_REQUIRE_EQUAL(self->state.received, 10u);
                    BOOST_REQUIRE(self->state.peer != invalid_connection_handle);
                    // delay new tokens to force MM to remove this broker from its loop
                    self->send(self, ok_atom::value);
                },
                [=](ok_atom) { self->become(server1_stage4(self)); }};
    }

    // consumes 5 tokens, then waits for passivated message and generates 5 more
    behavior server1_stage2(stateful_actor<server1_state, broker> *self) {
        BOOST_TEST_MESSAGE("enter server stage 2");
        self->trigger(self->state.peer, 5);
        return {[=](const new_data_msg &dm) {
                    BOOST_REQUIRE_EQUAL(dm.buf.size(), 10u);
                    self->state.received += 1;
                },
                [=](const connection_passivated_msg &cp) {
                    BOOST_REQUIRE(cp.handle == self->state.peer);
                    BOOST_REQUIRE_EQUAL(self->state.received, 5u);
                    BOOST_REQUIRE(self->state.peer != invalid_connection_handle);
                    self->become(server1_stage3(self));
                }};
    }

    // waits for the connection to the client
    behavior server1(stateful_actor<server1_state, broker> *self) {
        return {[=](const new_connection_msg &nc) {
            BOOST_REQUIRE(self->state.peer == invalid_connection_handle);
            self->state.peer = nc.handle;
            self->configure_read(nc.handle, receive_policy::exactly(10));
            self->become(server1_stage2(self));
        }};
    }

    // -- second test server -------------------------------------------------------

    struct server2_state {
        size_t accepted = 0;
    };

    // consumes 5 more tokens, then waits for passivated message to shutdown
    behavior server2_stage4(stateful_actor<server2_state, broker> *self) {
        BOOST_TEST_MESSAGE("enter server stage 4");
        return {[=](const new_connection_msg &) { self->state.accepted += 1; },
                [=](const acceptor_passivated_msg &) {
                    BOOST_REQUIRE_EQUAL(self->state.accepted, 16u);
                    BOOST_TEST_MESSAGE("server is done");
                    self->quit();
                }};
    }

    // consumes 5 more tokens, then waits for passivated message to send itself
    // a message that generates 5 more (force MM to actually remove this broker
    // from its event loop and then re-adding it)
    behavior server2_stage3(stateful_actor<server2_state, broker> *self) {
        BOOST_TEST_MESSAGE("enter server stage 3");
        return {[=](const new_connection_msg &) { self->state.accepted += 1; },
                [=](const acceptor_passivated_msg &cp) {
                    BOOST_REQUIRE_EQUAL(self->state.accepted, 11u);
                    // delay new tokens to force MM to remove this broker from its loop
                    self->send(self, ok_atom::value, cp.handle);
                },
                [=](ok_atom, accept_handle hdl) {
                    self->trigger(hdl, 5);
                    self->become(server2_stage4(self));
                }};
    }

    // consumes 5 tokens, then waits for passivated message and generates 5 more
    behavior server2_stage2(stateful_actor<server2_state, broker> *self) {
        BOOST_TEST_MESSAGE("enter server stage 2");
        BOOST_REQUIRE_EQUAL(self->state.accepted, 1u);
        return {[=](const new_connection_msg &) { self->state.accepted += 1; },
                [=](const acceptor_passivated_msg &cp) {
                    BOOST_REQUIRE_EQUAL(self->state.accepted, 6u);
                    self->trigger(cp.handle, 5);
                    self->become(server2_stage3(self));
                }};
    }

    // waits for the connection to the client
    behavior server2(stateful_actor<server2_state, broker> *self) {
        return {[=](const new_connection_msg &nc) {
            self->state.accepted += 1;
            self->trigger(nc.source, 5);
            self->become(server2_stage2(self));
        }};
    }

    // -- config and fixture -------------------------------------------------------

    struct config : spawner_config {
        config() {
            load<io::middleman>();
        }
    };

    struct fixture {
        config client_cfg;
        spawner client_system;
        config server_cfg;
        spawner server_system;

        fixture() : client_system(client_cfg), server_system(server_cfg) {
            // nop
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(trigger_tests, fixture)

BOOST_AUTO_TEST_CASE(trigger_connection_test) {
    BOOST_TEST_MESSAGE("spawn server");
    uint16_t port = 0;
    auto serv = server_system.middleman().spawn_server(server1, port);
    BOOST_REQUIRE(serv);
    BOOST_REQUIRE_NE(port, 0);
    BOOST_TEST_MESSAGE("server spawned at port " << port);
    std::thread child {[&] {
        auto cl = client_system.middleman().spawn_client(client, "localhost", port);
        BOOST_REQUIRE(cl);
    }};
    child.join();
}

BOOST_AUTO_TEST_CASE(trigger_acceptor_test) {
    BOOST_TEST_MESSAGE("spawn server");
    uint16_t port = 0;
    auto serv = server_system.middleman().spawn_server(server2, port);
    BOOST_REQUIRE(serv);
    BOOST_REQUIRE_NE(port, 0);
    BOOST_TEST_MESSAGE("server spawned at port " << port);
    std::thread child {[&] {
        // 16 clients will succeed to connect
        for (int i = 0; i < 16; ++i) {
            auto cl = client_system.middleman().spawn_client(client, "localhost", port);
            BOOST_REQUIRE(cl);
        }
    }};
    child.join();
}

BOOST_AUTO_TEST_SUITE_END()
