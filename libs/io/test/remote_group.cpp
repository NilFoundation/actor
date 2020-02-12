#include <nil/actor/config.hpp>

#define BOOST_TEST_MODULE io_dynamic_remote_group_test

#include <nil/actor/test/io_dsl.hpp>

#include <vector>
#include <algorithm>

#include <nil/actor/all.hpp>
#include <nil/actor/io/all.hpp>

using namespace nil::actor;

namespace {

    class config : public nil::actor::spawner_config {
    public:
        config() {
            load<nil::actor::io::middleman>();
            add_message_type<std::vector<actor>>("std::vector<actor>");
        }
    };

    const uint16_t port = 8080;

    const char *server = "mars";

    const char *group_name = "foobar";

    size_t received_messages = 0u;

    behavior group_receiver(event_based_actor *self) {
        self->set_default_handler(reflect_and_quit);
        return {[](ok_atom) { ++received_messages; }};
    }

    // Our server is `mars` and our client is `earth`.
    struct fixture : point_to_point_fixture<test_coordinator_fixture<config>> {
        fixture() {
            prepare_connection(mars, earth, server, port);
        }

        ~fixture() {
            for (auto &receiver : receivers) {
                anon_send_exit(receiver, exit_reason::user_shutdown);
            }
        }

        void spawn_receivers(planet_type &planet, group grp, size_t count) {
            for (size_t i = 0; i < count; ++i) {
                receivers.emplace_back(planet.sys.spawn_in_group(grp, group_receiver));
            }
        }

        std::vector<actor> receivers;
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(dynamic_remote_group_tests, fixture)

BOOST_AUTO_TEST_CASE(publish_local_groups_test) {
    loop_after_next_enqueue(mars);
    BOOST_CHECK_EQUAL(mars.sys.middleman().publish_local_groups(port), port);
}

BOOST_AUTO_TEST_CASE(connecting_to_remote_group) {
    BOOST_TEST_MESSAGE("publish local groups on mars");
    loop_after_next_enqueue(mars);
    BOOST_CHECK_EQUAL(mars.sys.middleman().publish_local_groups(port), port);
    BOOST_TEST_MESSAGE("call remote_group on earth");
    loop_after_next_enqueue(earth);
    auto grp = unbox(earth.mm.remote_group(group_name, server, port));
    BOOST_CHECK(grp);
    BOOST_CHECK_EQUAL(grp->get()->identifier(), group_name);
}

BOOST_AUTO_TEST_CASE(message_transmission, *boost::unit_test::disabled()) {
    BOOST_TEST_MESSAGE("spawn 5 receivers on mars");
    auto mars_grp = mars.sys.groups().get_local(group_name);
    spawn_receivers(mars, mars_grp, 5u);
    BOOST_TEST_MESSAGE("publish local groups on mars");
    loop_after_next_enqueue(mars);
    BOOST_CHECK_EQUAL(mars.sys.middleman().publish_local_groups(port), port);
    BOOST_TEST_MESSAGE("call remote_group on earth");
    loop_after_next_enqueue(earth);
    auto earth_grp = unbox(earth.mm.remote_group(group_name, server, port));
    BOOST_TEST_MESSAGE("spawn 5 more receivers on earth");
    spawn_receivers(earth, earth_grp, 5u);
    BOOST_TEST_MESSAGE("send message on mars and expect 10 handled messages total");
    {
        received_messages = 0u;
        scoped_actor self {mars.sys};
        self->send(mars_grp, ok_atom::value);

        exec_all();

        BOOST_CHECK_EQUAL(received_messages, 10u);
    }
    BOOST_TEST_MESSAGE("send message on earth and again expect 10 handled messages");
    {
        received_messages = 0u;
        scoped_actor self {earth.sys};
        self->send(earth_grp, ok_atom::value);

        exec_all();

        BOOST_CHECK_EQUAL(received_messages, 10u);
    }
}

BOOST_AUTO_TEST_SUITE_END()
