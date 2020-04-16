#include <nil/actor/config.hpp>

#define BOOST_TEST_MODULE io_dynamic_remote_actor_tcp_test

#include <nil/actor/test/dsl.hpp>

#include <vector>
#include <sstream>
#include <utility>
#include <algorithm>

#include <nil/actor/all.hpp>
#include <nil/actor/io/all.hpp>

using namespace nil::actor;

namespace {

    constexpr char local_host[] = "127.0.0.1";

    class config : public spawner_config {
    public:
        config() {
            load<io::middleman>();
            add_message_type<std::vector<int>>("std::vector<int>");
        }
    };

    struct fixture {
        // State for the server.
        config server_side_config;
        spawner server_side;
        io::middleman &server_side_mm;

        // State for the client.
        config client_side_config;
        spawner client_side;
        io::middleman &client_side_mm;

        fixture() :
            server_side(server_side_config), server_side_mm(server_side.middleman()), client_side(client_side_config),
            client_side_mm(client_side.middleman()) {
            // nop
        }
    };

    behavior make_pong_behavior() {
        return {[](int val) -> int {
            ++val;
            BOOST_TEST_MESSAGE("pong with " << val);
            return val;
        }};
    }

    behavior make_ping_behavior(event_based_actor *self, const actor &pong) {
        BOOST_TEST_MESSAGE("ping with " << 0);
        self->send(pong, 0);
        return {[=](int val) -> int {
            if (val == 3) {
                BOOST_TEST_MESSAGE("ping with exit");
                self->send_exit(self->current_sender(), exit_reason::user_shutdown);
                BOOST_TEST_MESSAGE("ping quits");
                self->quit();
            }
            BOOST_TEST_MESSAGE("ping with " << val);
            return val;
        }};
    }

    behavior make_sort_behavior() {
        return {[](std::vector<int> &vec) -> std::vector<int> {
            BOOST_TEST_MESSAGE("sorter received: " << deep_to_string(vec));
            std::sort(vec.begin(), vec.end());
            BOOST_TEST_MESSAGE("sorter sent: " << deep_to_string(vec));
            return std::move(vec);
        }};
    }

    behavior make_sort_requester_behavior(event_based_actor *self, const actor &sorter) {
        self->send(sorter, std::vector<int> {5, 4, 3, 2, 1});
        return {[=](const std::vector<int> &vec) {
            BOOST_TEST_MESSAGE("sort requester received: " << deep_to_string(vec));
            std::vector<int> expected_vec {1, 2, 3, 4, 5};
            BOOST_CHECK(vec == expected_vec);
            self->send_exit(sorter, exit_reason::user_shutdown);
            self->quit();
        }};
    }

    behavior fragile_mirror(event_based_actor *self) {
        return {[=](int i) {
            self->quit(exit_reason::user_shutdown);
            return i;
        }};
    }

    behavior linking_actor(event_based_actor *self, const actor &buddy) {
        BOOST_TEST_MESSAGE("link to mirror and send dummy message");
        self->link_to(buddy);
        self->send(buddy, 42);
        return {[](int i) { BOOST_CHECK_EQUAL(i, 42); }};
    }

}    // namespace

BOOST_FIXTURE_TEST_SUITE(dynamic_remote_actor_tests, fixture)

BOOST_AUTO_TEST_CASE(identity_semantics_tcp_test) {
    // server side
    auto server = server_side.spawn(make_pong_behavior);
    auto port1 = unbox(server_side_mm.publish(server, 0, local_host));
    auto port2 = unbox(server_side_mm.publish(server, 0, local_host));
    BOOST_REQUIRE_NE(port1, port2);
    auto same_server = unbox(server_side_mm.remote_actor(local_host, port2));
    BOOST_REQUIRE(same_server == server);
    BOOST_CHECK(same_server->node() == server_side.node());
    auto server1 = unbox(client_side_mm.remote_actor(local_host, port1));
    auto server2 = unbox(client_side_mm.remote_actor(local_host, port2));
    BOOST_CHECK(server1 == client_side_mm.remote_actor(local_host, port1));
    BOOST_CHECK(server2 == client_side_mm.remote_actor(local_host, port2));
    anon_send_exit(server, exit_reason::user_shutdown);
}

BOOST_AUTO_TEST_CASE(ping_pong_tcp_test) {
    // server side
    auto port = unbox(server_side_mm.publish(server_side.spawn(make_pong_behavior), 0, local_host));
    // client side
    auto pong = unbox(client_side_mm.remote_actor(local_host, port));
    client_side.spawn(make_ping_behavior, pong);
}

BOOST_AUTO_TEST_CASE(custom_message_type_tcp_test) {
    // server side
    auto port = unbox(server_side_mm.publish(server_side.spawn(make_sort_behavior), 0, local_host));
    // client side
    auto sorter = unbox(client_side_mm.remote_actor(local_host, port));
    client_side.spawn(make_sort_requester_behavior, sorter);
}

BOOST_AUTO_TEST_CASE(remote_link_tcp_test) {
    // server side
    auto port = unbox(server_side_mm.publish(server_side.spawn(fragile_mirror), 0, local_host));
    // client side
    auto mirror = unbox(client_side_mm.remote_actor(local_host, port));
    auto linker = client_side.spawn(linking_actor, mirror);
    scoped_actor self {client_side};
    self->wait_for(linker);
    BOOST_TEST_MESSAGE("linker exited");
    self->wait_for(mirror);
    BOOST_TEST_MESSAGE("mirror exited");
}

BOOST_AUTO_TEST_SUITE_END()
