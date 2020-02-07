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

#define BOOST_TEST_MODULE typed_response_promise_test

#include <nil/mtl/test/dsl.hpp>

#include <boost/test/unit_test.hpp>

#include <map>

#include <nil/mtl/config.hpp>
#include <nil/mtl/all.hpp>

using namespace nil::mtl;

namespace {

    using foo_actor = typed_actor<replies_to<int>::with<int>,
                                  replies_to<get_atom, int>::with<int>,
                                  replies_to<get_atom, int, int>::with<int, int>,
                                  replies_to<get_atom, double>::with<double>,
                                  replies_to<get_atom, double, double>::with<double, double>,
                                  reacts_to<put_atom, int, int>,
                                  reacts_to<put_atom, int, int, int>>;

    using foo_promise = typed_response_promise<int>;
    using foo2_promise = typed_response_promise<int, int>;
    using foo3_promise = typed_response_promise<double>;

    using get1_helper = typed_actor<replies_to<int, int>::with<put_atom, int, int>>;
    using get2_helper = typed_actor<replies_to<int, int, int>::with<put_atom, int, int, int>>;

    class foo_actor_impl : public foo_actor::base {
    public:
        foo_actor_impl(actor_config &cfg) : foo_actor::base(cfg) {
            // nop
        }

        behavior_type make_behavior() override {
            return {[=](int x) -> foo_promise {
                        auto resp = response(x * 2);
                        BOOST_CHECK(!resp.pending());
                        return resp.deliver(x * 4);    // has no effect
                    },
                    [=](get_atom, int x) -> foo_promise {
                        auto calculator = spawn([]() -> get1_helper::behavior_type {
                            return {[](int promise_id, int value) -> result<put_atom, int, int> {
                                return {put_atom::value, promise_id, value * 2};
                            }};
                        });
                        send(calculator, next_id_, x);
                        auto &entry = promises_[next_id_++];
                        entry = make_response_promise<foo_promise>();
                        return entry;
                    },
                    [=](get_atom, int x, int y) -> foo2_promise {
                        auto calculator = spawn([]() -> get2_helper::behavior_type {
                            return {[](int promise_id, int v0, int v1) -> result<put_atom, int, int, int> {
                                return {put_atom::value, promise_id, v0 * 2, v1 * 2};
                            }};
                        });
                        send(calculator, next_id_, x, y);
                        auto &entry = promises2_[next_id_++];
                        entry = make_response_promise<foo2_promise>();
                        // verify move semantics
                        BOOST_CHECK(entry.pending());
                        foo2_promise tmp(std::move(entry));
                        BOOST_CHECK(!entry.pending());
                        BOOST_CHECK(tmp.pending());
                        entry = std::move(tmp);
                        BOOST_CHECK(entry.pending());
                        BOOST_CHECK(!tmp.pending());
                        return entry;
                    },
                    [=](get_atom, double) -> foo3_promise {
                        auto resp = make_response_promise<double>();
                        return resp.deliver(make_error(sec::unexpected_message));
                    },
                    [=](get_atom, double x, double y) { return response(x * 2, y * 2); },
                    [=](put_atom, int promise_id, int x) {
                        auto i = promises_.find(promise_id);
                        if (i == promises_.end()) {
                            return;
                        }
                        i->second.deliver(x);
                        promises_.erase(i);
                    },
                    [=](put_atom, int promise_id, int x, int y) {
                        auto i = promises2_.find(promise_id);
                        if (i == promises2_.end()) {
                            return;
                        }
                        i->second.deliver(x, y);
                        promises2_.erase(i);
                    }};
        }

    private:
        int next_id_ = 0;
        std::map<int, foo_promise> promises_;
        std::map<int, foo2_promise> promises2_;
    };

    struct fixture {
        fixture() : system(cfg), self(system, true), foo(system.spawn<foo_actor_impl>()) {
            // nop
        }

        spawner_config cfg;
        spawner system;
        scoped_actor self;
        foo_actor foo;
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(typed_spawn_tests, fixture)

BOOST_AUTO_TEST_CASE(typed_response_promise_test) {
    typed_response_promise<int> resp;
    BOOST_TEST_MESSAGE("trigger 'invalid response promise' error");
    resp.deliver(1);    // delivers on an invalid promise has no effect
    auto f = make_function_view(foo);
    BOOST_CHECK_EQUAL(f(get_atom::value, 42), 84);
    BOOST_CHECK(f(get_atom::value, 42, 52) == std::make_tuple(84, 104));
    BOOST_CHECK(f(get_atom::value, 3.14, 3.14) == std::make_tuple(6.28, 6.28));
}

BOOST_AUTO_TEST_CASE(typed_response_promise_chained_test) {
    auto f = make_function_view(foo * foo * foo);
    BOOST_CHECK_EQUAL(f(1), 8);
}

// verify that only requests get an error response message
BOOST_AUTO_TEST_CASE(error_response_message_test_test) {
    auto f = make_function_view(foo);
    BOOST_CHECK(f(get_atom::value, 3.14) == sec::unexpected_message);
    self->send(foo, get_atom::value, 42);
    self->receive([](int x) { BOOST_CHECK_EQUAL(x, 84); },
                  [](double x) { BOOST_ERROR("unexpected ordinary response message received: " << x); });
    self->send(foo, get_atom::value, 3.14);
    self->receive([&](error &err) {
        BOOST_CHECK(err == sec::unexpected_message);
        self->send(self, message {});
    });
}

// verify that delivering to a satisfied promise has no effect
BOOST_AUTO_TEST_CASE(satisfied_promise_test) {
    self->send(foo, 1);
    self->send(foo, get_atom::value, 3.14, 3.14);
    int i = 0;
    self->receive_for(i, 2)([](int x) { BOOST_CHECK_EQUAL(x, 1 * 2); },
                            [](double x, double y) {
                                BOOST_CHECK_EQUAL(x, 3.14 * 2);
                                BOOST_CHECK_EQUAL(y, 3.14 * 2);
                            });
}

BOOST_AUTO_TEST_CASE(delegating_promises_test) {
    using task = std::pair<typed_response_promise<int>, int>;
    struct state {
        std::vector<task> tasks;
    };
    using bar_actor = typed_actor<replies_to<int>::with<int>, reacts_to<ok_atom>>;
    auto bar_fun = [](bar_actor::stateful_pointer<state> self, foo_actor worker) -> bar_actor::behavior_type {
        return {[=](int x) -> typed_response_promise<int> {
                    auto &tasks = self->state.tasks;
                    tasks.emplace_back(self->make_response_promise<int>(), x);
                    self->send(self, ok_atom::value);
                    return tasks.back().first;
                },
                [=](ok_atom) {
                    auto &tasks = self->state.tasks;
                    if (!tasks.empty()) {
                        auto &task = tasks.back();
                        task.first.delegate(worker, task.second);
                        tasks.pop_back();
                    }
                }};
    };
    auto f = make_function_view(system.spawn(bar_fun, foo));
    BOOST_CHECK_EQUAL(f(42), 84);
}

BOOST_AUTO_TEST_SUITE_END()
