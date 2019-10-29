//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt for Boost License or
// http://opensource.org/licenses/BSD-3-Clause for BSD 3-Clause License
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE actor_pool_test

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <nil/mtl/all.hpp>
#include <nil/mtl/config.hpp>

using namespace nil::mtl;

namespace {

    std::atomic<size_t> s_ctors;
    std::atomic<size_t> s_dtors;

    class worker : public event_based_actor {
    public:
        worker(actor_config &cfg) : event_based_actor(cfg) {
            ++s_ctors;
        }

        ~worker() override {
            ++s_dtors;
        }

        behavior make_behavior() override {
            auto nested = exit_handler_;
            set_exit_handler([=](scheduled_actor *self, exit_msg &em) { nested(self, em); });
            return {[](int x, int y) { return x + y; }};
        }
    };

    struct fixture {
        // allows us to check s_dtors after dtor of actor_system
        actor_system_config cfg;
        union {
            actor_system system;
        };
        union {
            scoped_execution_unit context;
        };

        std::function<actor()> spawn_worker;

        fixture() {
            new (&system) actor_system(cfg);
            new (&context) scoped_execution_unit(&system);
            spawn_worker = [&] { return system.spawn<worker>(); };
        }

        ~fixture() {
            system.await_all_actors_done();
            context.~scoped_execution_unit();
            system.~actor_system();
            BOOST_CHECK_EQUAL(s_dtors.load(), s_ctors.load());
        }
    };

    void handle_err(const error &err) {
        BOOST_FAIL("AUT responded with an error: " + to_string(err));
    }

}    // namespace

BOOST_FIXTURE_TEST_SUITE(actor_pool_tests, fixture)

BOOST_AUTO_TEST_CASE(round_robin_actor_pool_test) {
    scoped_actor self {system};
    auto pool = actor_pool::make(&context, 5, spawn_worker, actor_pool::round_robin());
    self->send(pool, sys_atom::value, put_atom::value, spawn_worker());
    std::vector<actor> workers;
    for (int i = 0; i < 6; ++i) {
        self->request(pool, infinite, i, i)
            .receive(
                [&](int res) {
                    BOOST_CHECK_EQUAL(res, i + i);
                    auto sender = actor_cast<strong_actor_ptr>(self->current_sender());
                    BOOST_REQUIRE(sender);
                    workers.push_back(actor_cast<actor>(std::move(sender)));
                },
                handle_err);
    }
    BOOST_CHECK_EQUAL(workers.size(), 6u);
    BOOST_CHECK(std::unique(workers.begin(), workers.end()) == workers.end());
    self->request(pool, infinite, sys_atom::value, get_atom::value)
        .receive(
            [&](std::vector<actor> &ws) {
                std::sort(workers.begin(), workers.end());
                std::sort(ws.begin(), ws.end());
                BOOST_REQUIRE_EQUAL(workers.size(), ws.size());
                BOOST_CHECK(std::equal(workers.begin(), workers.end(), ws.begin()));
            },
            handle_err);
    BOOST_TEST_MESSAGE("await last worker");
    anon_send_exit(workers.back(), exit_reason::user_shutdown);
    self->wait_for(workers.back());
    BOOST_TEST_MESSAGE("last worker shut down");
    workers.pop_back();
    // poll actor pool up to 10 times or until it removes the failed worker
    bool success = false;
    size_t i = 0;
    while (!success && ++i <= 10) {
        self->request(pool, infinite, sys_atom::value, get_atom::value)
            .receive(
                [&](std::vector<actor> &ws) {
                    success = workers.size() == ws.size();
                    if (success) {
                        std::sort(ws.begin(), ws.end());
                        BOOST_CHECK(workers == ws);
                    } else {
                        // wait a bit until polling again
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                },
                handle_err);
    }
    BOOST_REQUIRE_EQUAL(success, true);
    BOOST_TEST_MESSAGE("about to send exit to middleman_workers");
    self->send_exit(pool, exit_reason::user_shutdown);
    self->wait_for(workers);
}

BOOST_AUTO_TEST_CASE(broadcast_actor_pool_test) {
    scoped_actor self {system};
    auto spawn5 = [&] { return actor_pool::make(&context, 5, fixture::spawn_worker, actor_pool::broadcast()); };
    BOOST_CHECK_EQUAL(system.registry().running(), 1u);
    auto pool = actor_pool::make(&context, 5, spawn5, actor_pool::broadcast());
    BOOST_CHECK_EQUAL(system.registry().running(), 32u);
    self->send(pool, 1, 2);
    std::vector<int> results;
    int i = 0;
    self->receive_for(i, 25)([&](int res) { results.push_back(res); },
                             after(std::chrono::milliseconds(250)) >> [] { BOOST_ERROR("didn't receive a result"); });
    BOOST_CHECK_EQUAL(results.size(), 25u);
    BOOST_CHECK(std::all_of(results.begin(), results.end(), [](int res) { return res == 3; }));
    self->send_exit(pool, exit_reason::user_shutdown);
}

BOOST_AUTO_TEST_CASE(random_actor_pool_test) {
    scoped_actor self {system};
    auto pool = actor_pool::make(&context, 5, spawn_worker, actor_pool::random());
    for (int i = 0; i < 5; ++i) {
        self->request(pool, std::chrono::milliseconds(250), 1, 2)
            .receive([&](int res) { BOOST_CHECK_EQUAL(res, 3); }, handle_err);
    }
    self->send_exit(pool, exit_reason::user_shutdown);
}

BOOST_AUTO_TEST_CASE(split_join_actor_pool_test) {
    auto spawn_split_worker = [&] {
        return system.spawn<lazy_init>(
            []() -> behavior { return {[](size_t pos, const std::vector<int> &xs) { return xs[pos]; }}; });
    };
    auto split_fun = [](std::vector<std::pair<actor, message>> &xs, message &y) {
        for (size_t i = 0; i < xs.size(); ++i) {
            xs[i].second = make_message(i) + y;
        }
    };
    auto join_fun = [](int &res, message &msg) { msg.apply([&](int x) { res += x; }); };
    scoped_actor self {system};
    BOOST_TEST_MESSAGE("create actor pool");
    auto pool = actor_pool::make(&context, 5, spawn_split_worker, actor_pool::split_join<int>(join_fun, split_fun));
    self->request(pool, infinite, std::vector<int> {1, 2, 3, 4, 5})
        .receive([&](int res) { BOOST_CHECK_EQUAL(res, 15); }, handle_err);
    self->request(pool, infinite, std::vector<int> {6, 7, 8, 9, 10})
        .receive([&](int res) { BOOST_CHECK_EQUAL(res, 40); }, handle_err);
    self->send_exit(pool, exit_reason::user_shutdown);
}

BOOST_AUTO_TEST_SUITE_END()
