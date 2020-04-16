#include <nil/actor/config.hpp>

#define BOOST_TEST_MODULE io_typed_remote_actor_test

#include <nil/actor/test/dsl.hpp>

#include <thread>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <functional>

#include <nil/actor/all.hpp>
#include <nil/actor/io/all.hpp>

using namespace std;
using namespace nil::actor;

struct ping {
    int32_t value;
};

template<class Inspector>
typename Inspector::result_type inspect(Inspector &f, ping &x) {
    return f(meta::type_name("ping"), x.value);
}

bool operator==(const ping &lhs, const ping &rhs) {
    return lhs.value == rhs.value;
}

struct pong {
    int32_t value;
};

template<class Inspector>
typename Inspector::result_type inspect(Inspector &f, pong &x) {
    return f(meta::type_name("pong"), x.value);
}

bool operator==(const pong &lhs, const pong &rhs) {
    return lhs.value == rhs.value;
}

using server_type = typed_actor<replies_to<ping>::with<pong>>;

using client_type = typed_actor<>;

server_type::behavior_type server() {
    return {[](const ping &p) -> pong {
        BOOST_CHECK_EQUAL(p.value, 42);
        return pong {p.value};
    }};
}

void run_client(uint16_t port) {
    spawner_config cfg;
    cfg.load<io::middleman>().add_message_type<ping>("ping").add_message_type<pong>("pong");
    spawner sys {cfg};
    // check whether invalid_argument is thrown
    // when trying to connect to get an untyped
    // handle to the server
    auto res = sys.middleman().remote_actor("127.0.0.1", port);
    BOOST_REQUIRE(!res);
    BOOST_TEST_MESSAGE(sys.render(res.error()));
    BOOST_TEST_MESSAGE("connect to typed_remote_actor");
    auto serv = unbox(sys.middleman().remote_actor<server_type>("127.0.0.1", port));
    auto f = make_function_view(serv);
    BOOST_CHECK(f(ping {42}) == pong {42});
    anon_send_exit(serv, exit_reason::user_shutdown);
}

void run_server() {
    spawner_config cfg;
    cfg.load<io::middleman>().add_message_type<ping>("ping").add_message_type<pong>("pong");
    spawner sys {cfg};
    auto port = unbox(sys.middleman().publish(sys.spawn(server), 0, "127.0.0.1"));
    BOOST_REQUIRE(port != 0);
    BOOST_TEST_MESSAGE("running on port " << port << ", start client");
    std::thread child {[=] { run_client(port); }};
    child.join();
}

BOOST_AUTO_TEST_CASE(test_typed_remote_actor_test) {
    run_server();
}
