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

#define BOOST_TEST_MODULE splitter_test

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <nil/mtl/all.hpp>
#include <nil/mtl/config.hpp>

#define ERROR_HANDLER [&](error &err) { BOOST_FAIL(system.render(err)); }

using namespace nil::mtl;

namespace {

    using first_stage = typed_actor<replies_to<double>::with<double, double>>;
    using second_stage = typed_actor<replies_to<double, double>::with<double>, replies_to<double>::with<double>>;

    first_stage::behavior_type typed_first_stage() {
        return [](double x) { return std::make_tuple(x * 2.0, x * 4.0); };
    }

    second_stage::behavior_type typed_second_stage() {
        return {[](double x, double y) { return x * y; }, [](double x) { return 23.0 * x; }};
    }

    behavior untyped_first_stage() {
        return typed_first_stage().unbox();
    }

    behavior untyped_second_stage() {
        return typed_second_stage().unbox();
    }

    struct fixture {
        actor_system_config cfg;
        actor_system system;
        scoped_actor self;
        actor first;
        actor second;
        actor first_and_second;

        fixture() : system(cfg), self(system, true) {
            // nop
        }

        void init_untyped() {
            using namespace std::placeholders;
            first = system.spawn(untyped_first_stage);
            second = system.spawn(untyped_second_stage);
            first_and_second = splice(first, second);
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(sequencer_tests, fixture)

BOOST_AUTO_TEST_CASE(identity_test) {
    init_untyped();
    BOOST_CHECK(first != second);
    BOOST_CHECK(first != first_and_second);
    BOOST_CHECK(second != first_and_second);
}

BOOST_AUTO_TEST_CASE(kill_first_test) {
    init_untyped();
    anon_send_exit(first, exit_reason::kill);
    self->wait_for(first_and_second);
}

BOOST_AUTO_TEST_CASE(kill_second_test) {
    init_untyped();
    anon_send_exit(second, exit_reason::kill);
    self->wait_for(first_and_second);
}

BOOST_AUTO_TEST_CASE(untyped_splicing_test) {
    init_untyped();
    self->request(first_and_second, infinite, 42.0)
        .receive(
            [](double x, double y, double z) {
                BOOST_CHECK_EQUAL(x, (42.0 * 2.0));
                BOOST_CHECK_EQUAL(y, (42.0 * 4.0));
                BOOST_CHECK_EQUAL(z, (23.0 * 42.0));
            },
            ERROR_HANDLER);
}

BOOST_AUTO_TEST_SUITE_END()
