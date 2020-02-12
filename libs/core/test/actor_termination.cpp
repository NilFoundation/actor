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
// this suite tests whether actors terminate as expect in several use cases
#define BOOST_TEST_MODULE actor_termination_test

#include <nil/actor/test/dsl.hpp>

#include <nil/actor/all.hpp>
#include <nil/actor/config.hpp>

using namespace nil::actor;

namespace {

    behavior mirror_impl(event_based_actor *self) {
        self->set_default_handler(reflect);
        return [] {
            // nop
        };
    }

    struct fixture : test_coordinator_fixture<> {
        actor mirror;
        actor testee;

        fixture() {
            mirror = sys.spawn(mirror_impl);
            // run initialization code or mirror
            sched.run_once();
        }

        template<class... Ts>
        void spawn(Ts &&... xs) {
            testee = self->spawn(std::forward<Ts>(xs)...);
        }

        ~fixture() {
            self->wait_for(testee);
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(actor_termination_tests, fixture)

BOOST_AUTO_TEST_CASE(single_multiplexed_request_test) {
    auto f = [&](event_based_actor *self, actor server) {
        self->request(server, infinite, 42).then([=](int x) { BOOST_REQUIRE_EQUAL(x, 42); });
    };
    spawn(f, mirror);
    // run initialization code of testee
    sched.run_once();
    expect((int), from(testee).to(mirror).with(42));
    expect((int), from(mirror).to(testee).with(42));
}

BOOST_AUTO_TEST_CASE(multiple_multiplexed_requests_test) {
    auto f = [&](event_based_actor *self, actor server) {
        for (int i = 0; i < 3; ++i) {
            self->request(server, infinite, 42).then([=](int x) { BOOST_REQUIRE_EQUAL(x, 42); });
        }
    };
    spawn(f, mirror);
    // run initialization code of testee
    sched.run_once();
    expect((int), from(testee).to(mirror).with(42));
    expect((int), from(testee).to(mirror).with(42));
    expect((int), from(testee).to(mirror).with(42));
    expect((int), from(mirror).to(testee).with(42));
    expect((int), from(mirror).to(testee).with(42));
    expect((int), from(mirror).to(testee).with(42));
}

BOOST_AUTO_TEST_CASE(single_awaited_request_test) {
    auto f = [&](event_based_actor *self, actor server) {
        self->request(server, infinite, 42).await([=](int x) { BOOST_REQUIRE_EQUAL(x, 42); });
    };
    spawn(f, mirror);
    // run initialization code of testee
    sched.run_once();
    expect((int), from(testee).to(mirror).with(42));
    expect((int), from(mirror).to(testee).with(42));
}

BOOST_AUTO_TEST_CASE(multiple_awaited_requests_test) {
    auto f = [&](event_based_actor *self, actor server) {
        for (int i = 0; i < 3; ++i) {
            self->request(server, infinite, i).await([=](int x) {
                BOOST_TEST_MESSAGE("received response #" << (i + 1));
                BOOST_REQUIRE_EQUAL(x, i);
            });
        }
    };
    spawn(f, mirror);
    // run initialization code of testee
    sched.run_once();
    self->monitor(testee);
    expect((int), from(testee).to(mirror).with(0));
    expect((int), from(testee).to(mirror).with(1));
    expect((int), from(testee).to(mirror).with(2));
    // request().await() processes messages out-of-order,
    // which means we cannot check using expect()
    sched.run();
    expect((down_msg), from(testee).to(self).with(_));
}

BOOST_AUTO_TEST_SUITE_END()