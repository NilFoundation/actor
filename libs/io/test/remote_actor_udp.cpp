#include <nil/mtl/config.hpp>

#define BOOST_TEST_MODULE io_dynamic_remote_actor_udp_test

#include <nil/mtl/test/dsl.hpp>

#include <vector>
#include <sstream>
#include <utility>
#include <algorithm>

#include <nil/mtl/all.hpp>
#include <nil/mtl/io/all.hpp>

using namespace nil::mtl;

namespace {

    constexpr char local_host[] = "127.0.0.1";

    class config : public actor_system_config {
    public:
        config() {
            load<io::middleman>();
            enable_udp = true;
            add_message_type<std::vector<int>>("std::vector<int>");
        }
    };

    struct fixture {
        // State for the server.
        config server_side_config;
        actor_system server_side;
        io::middleman &server_side_mm;

        // State for the client.
        config client_side_config;
        actor_system client_side;
        io::middleman &client_side_mm;

        fixture() :
            server_side(server_side_config),
            server_side_mm(server_side.middleman()),
            client_side(client_side_config),
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

BOOST_FIXTURE_TEST_SUITE(dynamic_remote_actor_tests_udp, fixture)

BOOST_AUTO_TEST_CASE(identity_semantics_udp_test) {
    // server side
    auto server = server_side.spawn(make_pong_behavior);
    auto port1 = unbox(server_side_mm.publish_udp(server, 0, local_host));
    auto port2 = unbox(server_side_mm.publish_udp(server, 0, local_host));
    BOOST_REQUIRE_NE(port1, port2);
    auto same_server = unbox(server_side_mm.remote_actor_udp(local_host, port2));
    BOOST_REQUIRE(same_server == server);
    BOOST_CHECK(same_server->node() == server_side.node());
    auto server1 = unbox(client_side_mm.remote_actor_udp(local_host, port1));
    auto server2 = unbox(client_side_mm.remote_actor_udp(local_host, port2));
    BOOST_CHECK(server1 == client_side_mm.remote_actor_udp(local_host, port1));
    BOOST_CHECK(server2 == client_side_mm.remote_actor_udp(local_host, port2));
    anon_send_exit(server, exit_reason::user_shutdown);
}

BOOST_AUTO_TEST_CASE(ping_pong_udp_test) {
    // server side
    auto port = unbox(server_side_mm.publish_udp(server_side.spawn(make_pong_behavior), 0, local_host));
    // client side
    auto pong = unbox(client_side_mm.remote_actor_udp(local_host, port));
    client_side.spawn(make_ping_behavior, pong);
}

BOOST_AUTO_TEST_CASE(custom_message_type_udp_test) {
    // server side
    auto port = unbox(server_side_mm.publish_udp(server_side.spawn(make_sort_behavior), 0, local_host));
    // client side
    auto sorter = unbox(client_side_mm.remote_actor_udp(local_host, port));
    client_side.spawn(make_sort_requester_behavior, sorter);
}

BOOST_AUTO_TEST_CASE(remote_link_udp_test) {
    // server side
    auto port = unbox(server_side_mm.publish_udp(server_side.spawn(fragile_mirror), 0, local_host));
    // client side
    auto mirror = unbox(client_side_mm.remote_actor_udp(local_host, port));
    auto linker = client_side.spawn(linking_actor, mirror);
    scoped_actor self {client_side};
    self->wait_for(linker);
    BOOST_TEST_MESSAGE("linker exited");
    self->wait_for(mirror);
    BOOST_TEST_MESSAGE("mirror exited");
}

BOOST_AUTO_TEST_CASE(multiple_endpoints_udp_test) {
    config cfg;
    // Setup server.
    BOOST_TEST_MESSAGE("creating server");
    actor_system server_sys {cfg};
    auto mirror = server_sys.spawn([]() -> behavior {
        return {[](std::string str) {
            std::reverse(begin(str), end(str));
            return str;
        }};
    });
    auto port = server_sys.middleman().publish_udp(mirror, 0);
    BOOST_REQUIRE(port);
    BOOST_TEST_MESSAGE("server running on port " << port);
    auto client_fun = [](event_based_actor *self) -> behavior {
        return {[=](actor s) { self->send(s, "hellow, world"); },
                [=](const std::string &str) {
                    BOOST_CHECK_EQUAL(str, "dlrow ,wolleh");
                    self->quit();
                    BOOST_TEST_MESSAGE("done");
                }};
    };
    // Setup a client.
    BOOST_TEST_MESSAGE("creating first client");
    config client_cfg;
    actor_system client_sys {client_cfg};
    auto client = client_sys.spawn(client_fun);
    // Acquire remote actor from the server.
    auto client_srv = client_sys.middleman().remote_actor_udp("localhost", *port);
    BOOST_REQUIRE(client_srv);
    // Setup other clients.
    for (int i = 0; i < 5; ++i) {
        config other_cfg;
        actor_system other_sys {other_cfg};
        BOOST_TEST_MESSAGE("creating new client");
        auto other = other_sys.spawn(client_fun);
        // Acquire remote actor from the new server.
        auto other_srv = other_sys.middleman().remote_actor_udp("localhost", *port);
        BOOST_REQUIRE(other_srv);
        // Establish communication and exit.
        BOOST_TEST_MESSAGE("client contacts server and exits");
        anon_send(other, *other_srv);
        other_sys.await_all_actors_done();
    }
    // Start communicate from the first actor.
    BOOST_TEST_MESSAGE("first client contacts server and exits");
    anon_send(client, *client_srv);
    client_sys.await_all_actors_done();
    anon_send_exit(mirror, exit_reason::user_shutdown);
}

BOOST_AUTO_TEST_SUITE_END()
