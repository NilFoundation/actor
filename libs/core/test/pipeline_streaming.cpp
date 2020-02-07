//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt or
// http://opensource.org/licenses/BSD-3-Clause
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE pipeline_streaming_test

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <nil/mtl/test/dsl.hpp>

#include <memory>
#include <numeric>

#include <nil/mtl/spawner.hpp>
#include <nil/mtl/spawner_config.hpp>
#include <nil/mtl/event_based_actor.hpp>
#include <nil/mtl/stateful_actor.hpp>

using std::string;

using namespace nil::mtl;

namespace {

    TESTEE_SETUP();

    using buf = std::deque<int>;

    std::function<void(buf &)> init(size_t buf_size) {
        return [=](buf &xs) {
            xs.resize(buf_size);
            std::iota(xs.begin(), xs.end(), 1);
        };
    }

    void push_from_buf(buf &xs, downstream<int> &out, size_t num) {
        BOOST_TEST_MESSAGE("push " << num << " messages downstream");
        auto n = std::min(num, xs.size());
        for (size_t i = 0; i < n; ++i) {
            out.push(xs[i]);
        }
        xs.erase(xs.begin(), xs.begin() + static_cast<ptrdiff_t>(n));
    }

    std::function<bool(const buf &)> is_done(scheduled_actor *self) {
        return [=](const buf &xs) {
            if (xs.empty()) {
                BOOST_TEST_MESSAGE(self->name() << " exhausted its buffer");
                return true;
            }
            return false;
        };
    }

    template<class T>
    std::function<void(T &, const error &)> fin(scheduled_actor *self) {
        return [=](T &, const error &err) {
            if (err == none) {
                BOOST_TEST_MESSAGE(self->name() << " is done");
            } else {
                BOOST_TEST_MESSAGE(self->name() << " aborted with error");
            }
        };
    }

    TESTEE(infinite_source) {
        return {[=](string &fname) -> output_stream<int> {
            BOOST_CHECK_EQUAL(fname, "numbers.txt");
            BOOST_CHECK_EQUAL(self->mailbox().empty(), true);
            return self->make_source([](int &x) { x = 0; },
                                     [](int &x, downstream<int> &out, size_t num) {
                                         for (size_t i = 0; i < num; ++i) {
                                             out.push(x++);
                                         }
                                     },
                                     [](const int &) { return false; }, fin<int>(self));
        }};
    }

    VARARGS_TESTEE(file_reader, size_t buf_size) {
        return {[=](string &fname) -> output_stream<int> {
                    BOOST_CHECK_EQUAL(fname, "numbers.txt");
                    BOOST_CHECK_EQUAL(self->mailbox().empty(), true);
                    return self->make_source(init(buf_size), push_from_buf, is_done(self), fin<buf>(self));
                },
                [=](string &fname, actor next) {
                    BOOST_CHECK_EQUAL(fname, "numbers.txt");
                    BOOST_CHECK_EQUAL(self->mailbox().empty(), true);
                    self->make_source(next, init(buf_size), push_from_buf, is_done(self), fin<buf>(self));
                }};
    }

    TESTEE_STATE(sum_up) {
        int x = 0;
    };

    TESTEE(sum_up) {
        using intptr = int *;
        return {[=](stream<int> &in) {
            return self->make_sink(
                // input stream
                in,
                // initialize state
                [=](intptr &x) { x = &self->state.x; },
                // processing step
                [](intptr &x, int y) { *x += y; }, fin<intptr>(self));
        }};
    }

    TESTEE_STATE(delayed_sum_up) {
        int x = 0;
    };

    TESTEE(delayed_sum_up) {
        using intptr = int *;
        self->set_default_handler(skip);
        return {[=](ok_atom) {
            self->become([=](stream<int> &in) {
                self->set_default_handler(print_and_drop);
                return self->make_sink(
                    // input stream
                    in,
                    // initialize state
                    [=](intptr &x) { x = &self->state.x; },
                    // processing step
                    [](intptr &x, int y) { *x += y; },
                    // cleanup
                    fin<intptr>(self));
            });
        }};
    }

    TESTEE(broken_sink) {
        MTL_IGNORE_UNUSED(self);
        return {[=](stream<int> &, const actor &) {
            // nop
        }};
    }

    TESTEE(filter) {
        MTL_IGNORE_UNUSED(self);
        return {[=](stream<int> &in) {
            return self->make_stage(
                // input stream
                in,
                // initialize state
                [](unit_t &) {
                    // nop
                },
                // processing step
                [](unit_t &, downstream<int> &out, int x) {
                    if ((x & 0x01) != 0) {
                        out.push(x);
                    }
                },
                // cleanup
                fin<unit_t>(self));
        }};
    }

    TESTEE(doubler) {
        MTL_IGNORE_UNUSED(self);
        return {[=](stream<int> &in) {
            return self->make_stage(
                // input stream
                in,
                // initialize state
                [](unit_t &) {
                    // nop
                },
                // processing step
                [](unit_t &, downstream<int> &out, int x) { out.push(x * 2); },
                // cleanup
                fin<unit_t>(self));
        }};
    }

    struct fixture : test_coordinator_fixture<> {
        void tick() {
            advance_time(cfg.stream_credit_round_interval);
        }

        /// Simulate a hard error on an actor such as an uncaught exception or a
        /// disconnect from a remote actor.
        void hard_kill(const actor &x) {
            deref(x).cleanup(exit_reason::kill, nullptr);
        }
    };

}    // namespace

// -- unit tests ---------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(local_streaming_tests, fixture)

BOOST_AUTO_TEST_CASE(depth_2_pipeline_50_items_test) {
    auto src = sys.spawn(file_reader, 50u);
    auto snk = sys.spawn(sum_up);
    //    BOOST_TEST_MESSAGE(MTL_ARG(self) << MTL_ARG(src) << MTL_ARG(snk));
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(snk * src, "numbers.txt");
    expect((string), from(self).to(src).with("numbers.txt"));
    expect((open_stream_msg), from(self).to(snk));
    expect((upstream_msg::ack_open), from(snk).to(src));
    BOOST_TEST_MESSAGE("start data transmission (a single batch)");
    expect((downstream_msg::batch), from(src).to(snk));
    tick();
    expect((timeout_msg), from(snk).to(snk));
    expect((timeout_msg), from(src).to(src));
    expect((upstream_msg::ack_batch), from(snk).to(src));
    BOOST_TEST_MESSAGE("expect close message from src and then result from snk");
    expect((downstream_msg::close), from(src).to(snk));
    BOOST_CHECK_EQUAL(deref<sum_up_actor>(snk).state.x, 1275);
}

BOOST_AUTO_TEST_CASE(depth_2_pipeline_setup2_50_items_test) {
    auto src = sys.spawn(file_reader, 50u);
    auto snk = sys.spawn(sum_up);
    //    BOOST_TEST_MESSAGE(MTL_ARG(self) << MTL_ARG(src) << MTL_ARG(snk));
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(src, "numbers.txt", snk);
    expect((string, actor), from(self).to(src).with("numbers.txt", snk));
    expect((open_stream_msg), from(strong_actor_ptr {nullptr}).to(snk));
    expect((upstream_msg::ack_open), from(snk).to(src));
    BOOST_TEST_MESSAGE("start data transmission (a single batch)");
    expect((downstream_msg::batch), from(src).to(snk));
    tick();
    expect((timeout_msg), from(snk).to(snk));
    expect((timeout_msg), from(src).to(src));
    expect((upstream_msg::ack_batch), from(snk).to(src));
    BOOST_TEST_MESSAGE("expect close message from src and then result from snk");
    expect((downstream_msg::close), from(src).to(snk));
    BOOST_CHECK_EQUAL(deref<sum_up_actor>(snk).state.x, 1275);
}

BOOST_AUTO_TEST_CASE(delayed_depth_2_pipeline_50_items_test) {
    auto src = sys.spawn(file_reader, 50u);
    auto snk = sys.spawn(delayed_sum_up);
    //    BOOST_TEST_MESSAGE(MTL_ARG(self) << MTL_ARG(src) << MTL_ARG(snk));
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(snk * src, "numbers.txt");
    expect((string), from(self).to(src).with("numbers.txt"));
    expect((open_stream_msg), from(self).to(snk));
    disallow((upstream_msg::ack_open), from(snk).to(src));
    disallow((upstream_msg::forced_drop), from(_).to(src));
    BOOST_TEST_MESSAGE("send 'ok' to trigger sink to handle open_stream_msg");
    self->send(snk, ok_atom::value);
    expect((ok_atom), from(self).to(snk));
    expect((open_stream_msg), from(self).to(snk));
    expect((upstream_msg::ack_open), from(snk).to(src));
    BOOST_TEST_MESSAGE("start data transmission (a single batch)");
    expect((downstream_msg::batch), from(src).to(snk));
    tick();
    expect((timeout_msg), from(snk).to(snk));
    expect((timeout_msg), from(src).to(src));
    expect((upstream_msg::ack_batch), from(snk).to(src));
    BOOST_TEST_MESSAGE("expect close message from src and then result from snk");
    expect((downstream_msg::close), from(src).to(snk));
    BOOST_CHECK_EQUAL(deref<delayed_sum_up_actor>(snk).state.x, 1275);
}

BOOST_AUTO_TEST_CASE(depth_2_pipeline_500_items_test) {
    auto src = sys.spawn(file_reader, 500u);
    auto snk = sys.spawn(sum_up);
    //    BOOST_TEST_MESSAGE(MTL_ARG(self) << MTL_ARG(src) << MTL_ARG(snk));
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(snk * src, "numbers.txt");
    expect((string), from(self).to(src).with("numbers.txt"));
    expect((open_stream_msg), from(self).to(snk));
    expect((upstream_msg::ack_open), from(snk).to(src));
    BOOST_TEST_MESSAGE("start data transmission (loop until src sends 'close')");
    do {
        BOOST_TEST_MESSAGE("process all batches at the sink");
        while (received<downstream_msg::batch>(snk)) {
            expect((downstream_msg::batch), from(src).to(snk));
        }
        BOOST_TEST_MESSAGE("trigger timeouts");
        tick();
        allow((timeout_msg), from(snk).to(snk));
        allow((timeout_msg), from(src).to(src));
        BOOST_TEST_MESSAGE("process ack_batch in source");
        expect((upstream_msg::ack_batch), from(snk).to(src));
    } while (!received<downstream_msg::close>(snk));
    BOOST_TEST_MESSAGE("expect close message from src and then result from snk");
    expect((downstream_msg::close), from(src).to(snk));
    BOOST_CHECK_EQUAL(deref<sum_up_actor>(snk).state.x, 125250);
}

BOOST_AUTO_TEST_CASE(depth_2_pipeline_error_during_handshake_test) {
    BOOST_TEST_MESSAGE("streams must abort if a sink fails to initialize its state");
    auto src = sys.spawn(file_reader, 50u);
    auto snk = sys.spawn(broken_sink);
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(snk * src, "numbers.txt");
    expect((std::string), from(self).to(src).with("numbers.txt"));
    expect((open_stream_msg), from(self).to(snk));
    expect((upstream_msg::forced_drop), from(_).to(src));
    expect((error), from(snk).to(self).with(sec::stream_init_failed));
}

BOOST_AUTO_TEST_CASE(depth_2_pipeline_error_at_source_test) {
    BOOST_TEST_MESSAGE("streams must abort if a source fails at runtime");
    auto src = sys.spawn(file_reader, 500u);
    auto snk = sys.spawn(sum_up);
    //    BOOST_TEST_MESSAGE(MTL_ARG(self) << MTL_ARG(src) << MTL_ARG(snk));
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(snk * src, "numbers.txt");
    expect((string), from(self).to(src).with("numbers.txt"));
    expect((open_stream_msg), from(self).to(snk));
    expect((upstream_msg::ack_open), from(snk).to(src));
    BOOST_TEST_MESSAGE("start data transmission (and abort source)");
    hard_kill(src);
    expect((downstream_msg::batch), from(src).to(snk));
    expect((downstream_msg::forced_close), from(_).to(snk));
}

BOOST_AUTO_TEST_CASE(depth_2_pipelin_error_at_sink_test) {
    BOOST_TEST_MESSAGE("streams must abort if a sink fails at runtime");
    auto src = sys.spawn(file_reader, 500u);
    auto snk = sys.spawn(sum_up);
    //    BOOST_TEST_MESSAGE(MTL_ARG(self) << MTL_ARG(src) << MTL_ARG(snk));
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(snk * src, "numbers.txt");
    expect((string), from(self).to(src).with("numbers.txt"));
    expect((open_stream_msg), from(self).to(snk));
    BOOST_TEST_MESSAGE("start data transmission (and abort sink)");
    hard_kill(snk);
    expect((upstream_msg::ack_open), from(snk).to(src));
    expect((upstream_msg::forced_drop), from(_).to(src));
}

BOOST_AUTO_TEST_CASE(depth_3_pipeline_50_items_test) {
    auto src = sys.spawn(file_reader, 50u);
    auto stg = sys.spawn(filter);
    auto snk = sys.spawn(sum_up);
    auto next_cycle = [&] {
        tick();
        allow((timeout_msg), from(snk).to(snk));
        allow((timeout_msg), from(stg).to(stg));
        allow((timeout_msg), from(src).to(src));
    };
    //    BOOST_TEST_MESSAGE(MTL_ARG(self) << MTL_ARG(src) << MTL_ARG(stg) << MTL_ARG(snk));
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(snk * stg * src, "numbers.txt");
    expect((string), from(self).to(src).with("numbers.txt"));
    expect((open_stream_msg), from(self).to(stg));
    expect((open_stream_msg), from(self).to(snk));
    expect((upstream_msg::ack_open), from(snk).to(stg));
    expect((upstream_msg::ack_open), from(stg).to(src));
    BOOST_TEST_MESSAGE("start data transmission (a single batch)");
    expect((downstream_msg::batch), from(src).to(stg));
    BOOST_TEST_MESSAGE("the stage should delay its first batch since its underfull");
    disallow((downstream_msg::batch), from(stg).to(snk));
    next_cycle();
    BOOST_TEST_MESSAGE("the source shuts down and the stage sends the final batch");
    expect((upstream_msg::ack_batch), from(stg).to(src));
    expect((downstream_msg::close), from(src).to(stg));
    expect((downstream_msg::batch), from(stg).to(snk));
    next_cycle();
    BOOST_TEST_MESSAGE("the stage shuts down and the sink produces its final result");
    expect((upstream_msg::ack_batch), from(snk).to(stg));
    expect((downstream_msg::close), from(stg).to(snk));
    BOOST_CHECK_EQUAL(deref<sum_up_actor>(snk).state.x, 625);
}

BOOST_AUTO_TEST_CASE(depth_4_pipeline_500_items_test) {
    auto src = sys.spawn(file_reader, 500u);
    auto stg1 = sys.spawn(filter);
    auto stg2 = sys.spawn(doubler);
    auto snk = sys.spawn(sum_up);
    //    BOOST_TEST_MESSAGE(MTL_ARG(self) << MTL_ARG(src) << MTL_ARG(stg1) << MTL_ARG(stg2) << MTL_ARG(snk));
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(snk * stg2 * stg1 * src, "numbers.txt");
    expect((string), from(self).to(src).with("numbers.txt"));
    expect((open_stream_msg), from(self).to(stg1));
    expect((open_stream_msg), from(self).to(stg2));
    expect((open_stream_msg), from(self).to(snk));
    expect((upstream_msg::ack_open), from(snk).to(stg2));
    expect((upstream_msg::ack_open), from(stg2).to(stg1));
    expect((upstream_msg::ack_open), from(stg1).to(src));
    BOOST_TEST_MESSAGE("start data transmission");
    run();
    BOOST_TEST_MESSAGE("check sink result");
    BOOST_CHECK_EQUAL(deref<sum_up_actor>(snk).state.x, 125000);
}

BOOST_AUTO_TEST_CASE(depth_3_pipeline_graceful_shutdown_test) {
    auto src = sys.spawn(file_reader, 50u);
    auto stg = sys.spawn(filter);
    auto snk = sys.spawn(sum_up);
    //    BOOST_TEST_MESSAGE(MTL_ARG(self) << MTL_ARG(src) << MTL_ARG(stg) << MTL_ARG(snk));
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(snk * stg * src, "numbers.txt");
    expect((string), from(self).to(src).with("numbers.txt"));
    expect((open_stream_msg), from(self).to(stg));
    expect((open_stream_msg), from(self).to(snk));
    expect((upstream_msg::ack_open), from(snk).to(stg));
    expect((upstream_msg::ack_open), from(stg).to(src));
    BOOST_TEST_MESSAGE("start data transmission (a single batch) and stop the stage");
    anon_send_exit(stg, exit_reason::user_shutdown);
    BOOST_TEST_MESSAGE("expect the stage to still transfer pending items to the sink");
    run();
    BOOST_TEST_MESSAGE("check sink result");
    BOOST_CHECK_EQUAL(deref<sum_up_actor>(snk).state.x, 625);
}

BOOST_AUTO_TEST_CASE(depth_3_pipeline_infinite_source_test) {
    auto src = sys.spawn(infinite_source);
    auto stg = sys.spawn(filter);
    auto snk = sys.spawn(sum_up);
    //    BOOST_TEST_MESSAGE(MTL_ARG(self) << MTL_ARG(src) << MTL_ARG(stg) << MTL_ARG(snk));
    BOOST_TEST_MESSAGE("initiate stream handshake");
    self->send(snk * stg * src, "numbers.txt");
    expect((string), from(self).to(src).with("numbers.txt"));
    expect((open_stream_msg), from(self).to(stg));
    expect((open_stream_msg), from(self).to(snk));
    expect((upstream_msg::ack_open), from(snk).to(stg));
    expect((upstream_msg::ack_open), from(stg).to(src));
    BOOST_TEST_MESSAGE("send exit to the source and expect the stream to terminate");
    anon_send_exit(src, exit_reason::user_shutdown);
    run();
}

BOOST_AUTO_TEST_SUITE_END()
