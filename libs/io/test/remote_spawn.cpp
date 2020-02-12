#include <nil/actor/config.hpp>

#define BOOST_TEST_MODULE io_remote_spawn_test

#include <nil/actor/test/dsl.hpp>

#include <thread>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <functional>

#include <nil/actor/all.hpp>
#include <nil/actor/io/all.hpp>

using namespace nil::actor;

namespace {

    using add_atom = atom_constant<atom("add")>;
    using sub_atom = atom_constant<atom("sub")>;

    using calculator =
        typed_actor<replies_to<add_atom, int, int>::with<int>, replies_to<sub_atom, int, int>::with<int>>;

    // function-based, dynamically typed, event-based API
    behavior calculator_fun(event_based_actor *) {
        return behavior {[](add_atom, int a, int b) { return a + b; }, [](sub_atom, int a, int b) { return a - b; }};
    }

    // function-based, statically typed, event-based API
    calculator::behavior_type typed_calculator_fun() {
        return {[](add_atom, int a, int b) { return a + b; }, [](sub_atom, int a, int b) { return a - b; }};
    }

    struct config : spawner_config {
        config() {
            load<io::middleman>();
            add_actor_type("calculator", calculator_fun);
            add_actor_type("typed_calculator", typed_calculator_fun);
        }
    };

    void run_client(uint16_t port) {
        config cfg {};
        spawner system {cfg};
        auto &mm = system.middleman();
        auto nid = mm.connect("localhost", port);
        BOOST_REQUIRE(nid);
        BOOST_REQUIRE(system.node() != *nid);
        auto calc = mm.remote_spawn<calculator>(*nid, "calculator", make_message());
        BOOST_REQUIRE(!calc);
        BOOST_REQUIRE(calc.error().category() == atom("system"));
        BOOST_REQUIRE(static_cast<sec>(calc.error().code()) == sec::unexpected_actor_messaging_interface);
        calc = mm.remote_spawn<calculator>(*nid, "typed_calculator", make_message());
        BOOST_REQUIRE(calc);
        auto f1 = make_function_view(*calc);
        BOOST_REQUIRE_EQUAL(f1(add_atom::value, 10, 20), 30);
        BOOST_REQUIRE_EQUAL(f1(sub_atom::value, 10, 20), -10);
        f1.reset();
        anon_send_exit(*calc, exit_reason::kill);
        mm.close(port);
    }

    void run_server() {
        config cfg {};
        spawner system {cfg};
        auto port = unbox(system.middleman().open(0));
        std::thread child {[=] { run_client(port); }};
        child.join();
    }

}    // namespace

BOOST_AUTO_TEST_CASE(remote_spawn_test) {
    run_server();
}
