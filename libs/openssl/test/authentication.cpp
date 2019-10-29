#include <nil/mtl/config.hpp>

#define BOOST_TEST_MODULE openssl_authentication_test

#include <nil/mtl/test/dsl.hpp>

#ifndef MTL_WINDOWS

#include <unistd.h>

#else
#include <io.h>
#include <windows.h>
#define F_OK 0
#define PATH_MAX MAX_PATH
#endif

#include <vector>
#include <sstream>
#include <utility>
#include <algorithm>

#include <limits.h>
#include <stdlib.h>

#include <nil/mtl/all.hpp>
#include <nil/mtl/io/all.hpp>
#include <nil/mtl/openssl/all.hpp>

using namespace nil::mtl;

namespace {

    constexpr char local_host[] = "127.0.0.1";

    class config : public actor_system_config {
    public:
        config() {
            load<io::middleman>();
            load<openssl::manager>();
            add_message_type<std::vector<int>>("std::vector<int>");
            actor_system_config::parse(test::engine::argc(), test::engine::argv());
            set("middleman.manual-multiplexing", true);
            set("middleman.attach-utility-actors", true);
            set("scheduler.policy", atom("testing"));
        }

        static std::string data_dir() {
            std::string path {::nil::mtl::test::engine::path()};
            path = path.substr(0, path.find_last_of("/"));
            // TODO: https://github.com/actor-framework/actor-framework/issues/555
            path += "/../../openssl/test";
            char rpath[PATH_MAX];
#ifndef MTL_WINDOWS
            auto rp = realpath(path.c_str(), rpath);
#else
            auto rp = GetFullPathName(path.c_str(), PATH_MAX, rpath, nullptr);
#endif
            std::string result;
            if (rp) {
                result = rpath;
            }
            return result;
        }
    };

    behavior make_pong_behavior() {
        return {[](int val) -> int {
            ++val;
            BOOST_TEST_MESSAGE("pong " << val);
            return val;
        }};
    }

    behavior make_ping_behavior(event_based_actor *self, const actor &pong) {
        BOOST_TEST_MESSAGE("ping " << 0);
        self->send(pong, 0);
        return {[=](int val) -> int {
            BOOST_TEST_MESSAGE("ping " << val);
            if (val >= 3) {
                BOOST_TEST_MESSAGE("terminate ping");
                self->quit();
            }
            return val;
        }};
    }

    struct fixture {
        using sched_t = scheduler::test_coordinator;

        config server_side_config;
        config client_side_config;
        bool initialized;
        union {
            actor_system server_side;
        };
        union {
            actor_system client_side;
        };
        sched_t *ssched;
        sched_t *csched;

        fixture() : initialized(false) {
            // nop
        }

        ~fixture() {
            if (initialized) {
                server_side.~actor_system();
                client_side.~actor_system();
            }
        }

        bool init(bool skip_client_side_ca) {
            auto cd = config::data_dir();
            cd += '/';
            server_side_config.openssl_passphrase = "12345";
            // check whether all files exist before setting config parameters
            std::string dummy;
            std::pair<const char *, std::string *> cfg[] {
                {"ca.pem", &server_side_config.openssl_cafile},
                {"cert.1.pem", &server_side_config.openssl_certificate},
                {"key.1.enc.pem", &server_side_config.openssl_key},
                {"ca.pem", skip_client_side_ca ? &dummy : &client_side_config.openssl_cafile},
                {"cert.2.pem", &client_side_config.openssl_certificate},
                {"key.2.pem", &client_side_config.openssl_key}};
            // return if any file is unreadable or non-existend
            for (auto &x : cfg) {
                auto path = cd + x.first;
                if (access(path.c_str(), F_OK) == -1) {
                    BOOST_TEST_MESSAGE("pem files missing, skip test");
                    return false;
                }
                *x.second = std::move(path);
            }
            BOOST_TEST_MESSAGE("initialize server side");
            new (&server_side) actor_system(server_side_config);
            BOOST_TEST_MESSAGE("initialize client side");
            new (&client_side) actor_system(client_side_config);
            ssched = &dynamic_cast<sched_t &>(server_side.scheduler());
            csched = &dynamic_cast<sched_t &>(client_side.scheduler());
            initialized = true;
            return true;
        }

        sched_t &sched_by_sys(actor_system &sys) {
            return &sys == &server_side ? *ssched : *csched;
        }

        bool exec_one(actor_system &sys) {
            MTL_ASSERT(initialized);
            MTL_PUSH_AID(0);
            MTL_SET_LOGGER_SYS(&sys);
            return sched_by_sys(sys).try_run_once() || sys.middleman().backend().try_run_once();
        }

        void exec_loop(actor_system &sys) {
            while (exec_one(sys)) {
            }    // nop
        }

        void exec_loop() {
            while (exec_one(client_side) | exec_one(server_side)) {
            }    // nop
        }

        void loop_after_next_enqueue(actor_system &sys) {
            auto s = &sys == &server_side ? ssched : csched;
            s->after_next_enqueue([=] { exec_loop(); });
        }

        bool terminated(const actor &x) {
            return x ? x->getf(abstract_actor::is_terminated_flag) : false;
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(authentication, fixture)

using openssl::publish;
using openssl::remote_actor;

BOOST_AUTO_TEST_CASE(authentication_success_test) {
    if (!init(false)) {
        return;
    }
    // server side
    BOOST_TEST_MESSAGE("spawn pong on server");
    auto spong = server_side.spawn(make_pong_behavior);
    exec_loop();
    BOOST_TEST_MESSAGE("publish pong");
    loop_after_next_enqueue(server_side);
    auto port = unbox(publish(spong, 0, local_host));
    exec_loop();
    // client side
    BOOST_TEST_MESSAGE("connect to pong via port " << port);
    loop_after_next_enqueue(client_side);
    auto pong = unbox(remote_actor(client_side, local_host, port));
    BOOST_TEST_MESSAGE("spawn ping and exchange messages");
    auto sping = client_side.spawn(make_ping_behavior, pong);
    while (!terminated(sping)) {
        exec_loop();
    }
    BOOST_TEST_MESSAGE("terminate pong");
    anon_send_exit(spong, exit_reason::user_shutdown);
    exec_loop();
}

BOOST_AUTO_TEST_CASE(authentication_failure_test) {
    if (!init(true)) {
        return;
    }
    // server side
    BOOST_TEST_MESSAGE("spawn pong on server");
    auto spong = server_side.spawn(make_pong_behavior);
    exec_loop();
    loop_after_next_enqueue(server_side);
    BOOST_TEST_MESSAGE("publish pong");
    auto port = unbox(publish(spong, 0, local_host));
    exec_loop();
    // client side
    BOOST_TEST_MESSAGE("connect to pong via port " << port);
    loop_after_next_enqueue(client_side);
    auto remote_pong = remote_actor(client_side, local_host, port);
    BOOST_CHECK(!remote_pong);
    BOOST_TEST_MESSAGE("terminate pong");
    anon_send_exit(spong, exit_reason::user_shutdown);
    exec_loop();
}

BOOST_AUTO_TEST_SUITE_END()
