/*
 * MIT License
 *
 * Copyright (c) 2018 Hippolyte Barraud
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <ultramarine/actor.hpp>
#include <ultramarine/actor_ref.hpp>
#include <ultramarine/utility.hpp>
#include "benchmark_utility.hpp"

static constexpr std::size_t NumMessage = 1000;
static constexpr std::size_t SenderCount = 1000;

class receiver : public ultramarine::actor<receiver> {
public:
ULTRAMARINE_DEFINE_ACTOR(receiver, (receive));
    std::size_t received = 0;

    void receive() {
        ++received;
    };
};

class sender : public ultramarine::actor<sender> {
public:
ULTRAMARINE_DEFINE_ACTOR(sender, (send));
    std::size_t sent = 0;

    nil::actor::future<> send(ultramarine::actor_id whom) {
        return nil::actor::do_with(ultramarine::get<receiver>(whom), [this](auto const &whom) {
            return nil::actor::do_until([this] { return sent++ >= NumMessage; }, [&whom] {
                return whom->receive();
            });
        });
    };
};

thread_local static int i;

nil::actor::future<> mailbox_performance() {
    i = 0;
    return sender::clear_directory().then([] {
        return ultramarine::with_buffer(SenderCount, [](auto &buffer) {
            return nil::actor::do_until([] { return i >= SenderCount; }, [&buffer] {
                return buffer(ultramarine::get<sender>(i++)->send(0));
            });
        });
    });
}

int main(int ac, char **av) {
    return ultramarine::benchmark::run(ac, av, {
            ULTRAMARINE_BENCH(mailbox_performance)
    }, 10);
}
