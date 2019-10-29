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

#define BOOST_TEST_MODULE delayed_send_test

#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/behavior.hpp>
#include <nil/mtl/event_based_actor.hpp>

#include <nil/mtl/test/dsl.hpp>

using namespace nil::mtl;

using std::chrono::seconds;

namespace {

    behavior testee_impl(event_based_actor *self) {
        self->set_default_handler(drop);
        return {[] {
            // nop
        }};
    }

}    // namespace

BOOST_FIXTURE_TEST_SUITE(request_timeout_tests, test_coordinator_fixture<>)

BOOST_AUTO_TEST_CASE(delayed_actor_message_test) {
    auto testee = sys.spawn(testee_impl);
    self->delayed_send(testee, seconds(1), "hello world");
    sched.

        trigger_timeout();

    expect((std::string), from(self).to(testee).with("hello world"));
}

BOOST_AUTO_TEST_CASE(delayed_group_message_test) {
    auto grp = sys.groups().anonymous();
    auto testee = sys.spawn_in_group(grp, testee_impl);
    self->delayed_send(grp, seconds(1), "hello world");
    sched.

        trigger_timeout();

    expect((std::string), from(self).to(testee).with("hello world"));
    // The group keeps a reference, so we need to shutdown 'manually'.
    anon_send_exit(testee, exit_reason::user_shutdown);
}

BOOST_AUTO_TEST_SUITE_END()
