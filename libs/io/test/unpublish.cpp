#include <nil/mtl/config.hpp>

#define BOOST_TEST_MODULE io_unpublish_test

#include <nil/mtl/test/dsl.hpp>

#include <new>
#include <thread>
#include <atomic>

#include <nil/mtl/all.hpp>
#include <nil/mtl/io/all.hpp>

using namespace nil::mtl;

namespace {

    std::atomic<long> s_dtor_called;

    class dummy : public event_based_actor {
    public:
        dummy(actor_config &cfg) : event_based_actor(cfg) {
            // nop
        }

        ~dummy() override {
            ++s_dtor_called;
        }

        behavior make_behavior() override {
            return {[] {
                // nop
            }};
        }
    };

    struct fixture {
        fixture() {
            new (&system) actor_system(cfg.load<io::middleman>());
            testee = system.spawn<dummy>();
        }

        ~fixture() {
            anon_send_exit(testee, exit_reason::user_shutdown);
            destroy(testee);
            system.~actor_system();
            BOOST_CHECK_EQUAL(s_dtor_called.load(), 2);
        }

        actor remote_actor(const char *hostname, uint16_t port, bool expect_fail = false) {
            actor result;
            scoped_actor self {system, true};
            self->request(system.middleman().actor_handle(), infinite, connect_atom::value, hostname, port)
                .receive(
                    [&](node_id &, strong_actor_ptr &res, std::set<std::string> &xs) {
                        BOOST_REQUIRE(xs.empty());
                        if (res) {
                            result = actor_cast<actor>(std::move(res));
                        }
                    },
                    [&](error &) {
                        // nop
                    });
            if (expect_fail) {
                BOOST_REQUIRE(!result);
            } else {
                BOOST_REQUIRE(result);
            }
            return result;
        }

        actor_system_config cfg;
        union {
            actor_system system;
        };    // manually control ctor/dtor
        actor testee;
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(unpublish_tests, fixture)

BOOST_AUTO_TEST_CASE(unpublishing_test) {
    auto port = unbox(system.middleman().publish(testee, 0));
    BOOST_REQUIRE(port != 0);
    BOOST_TEST_MESSAGE("published actor on port " << port);
    BOOST_TEST_MESSAGE("test invalid unpublish");
    auto testee2 = system.spawn<dummy>();
    system.middleman().unpublish(testee2, port);
    auto x0 = remote_actor("127.0.0.1", port);
    BOOST_CHECK(x0 != testee2);
    BOOST_CHECK(x0 == testee);
    anon_send_exit(testee2, exit_reason::kill);
    BOOST_TEST_MESSAGE("unpublish testee");
    system.middleman().unpublish(testee, port);
    BOOST_TEST_MESSAGE("check whether testee is still available via cache");
    auto x1 = remote_actor("127.0.0.1", port);
    BOOST_CHECK(x1 == testee);
    BOOST_TEST_MESSAGE("fake death of testee and check if testee becomes unavailable");
    anon_send(actor_cast<actor>(system.middleman().actor_handle()), down_msg {testee.address(), exit_reason::normal});
    // must fail now
    auto x2 = remote_actor("127.0.0.1", port, true);
    BOOST_CHECK(!x2);
}

BOOST_AUTO_TEST_SUITE_END()
