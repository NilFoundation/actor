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

static constexpr std::size_t PingPongCount = 10000000;

class ping_actor : public ultramarine::actor<ping_actor> {
public:
ULTRAMARINE_DEFINE_ACTOR(ping_actor, (ping_pong));
    std::size_t pingpong_count = 0;

    nil::actor::future<> ping_pong(int pong_addr);
};

class pong_actor : public ultramarine::actor<pong_actor> {
public:
ULTRAMARINE_DEFINE_ACTOR(pong_actor, (pong));

    void pong() const;
};

nil::actor::future<> ping_actor::ping_pong(int pong_addr) {
    pingpong_count = 0;
    auto pong = ultramarine::get<pong_actor>(pong_addr);
    return nil::actor::do_until([this] { return pingpong_count >= PingPongCount; }, [this, pong] {
        return pong->pong().then([this] {
            ++pingpong_count;
        });
    });
}

void pong_actor::pong() const {}

nil::actor::future<> pingpong_collocated() {
    return ultramarine::get<ping_actor>(0)->ping_pong(1);
}

int main(int ac, char **av) {
    return ultramarine::benchmark::run(ac, av, {
            ULTRAMARINE_BENCH(pingpong_collocated)
    }, 10);
}
