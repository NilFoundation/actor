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
#include "benchmark_utility.hpp"

static constexpr std::size_t PhilosopherLen = 10;
static constexpr std::size_t RoundLen = 1000;

class arbitrator_actor : public ultramarine::actor<arbitrator_actor> {
public:
ULTRAMARINE_DEFINE_ACTOR(arbitrator_actor, (hungry)(done));
    std::array<bool, PhilosopherLen> forks{};

    nil::actor::future<bool> hungry(int philosopher_index) {
        auto &leftFork = forks[philosopher_index];
        auto &rightFork = forks[(philosopher_index + 1) % PhilosopherLen];

        if (leftFork || rightFork) {
            return nil::actor::make_ready_future<bool>(false);
        } else {
            return nil::actor::make_ready_future<bool>(leftFork = rightFork = true);
        }
    }

    void done(int philosopher_index) {
        forks[philosopher_index] = false;
        forks[(philosopher_index + 1) % PhilosopherLen] = false;
    }
};

std::atomic<int> failed_attempts = 0;

class philosopher_actor : public ultramarine::actor<philosopher_actor> {
public:
ULTRAMARINE_DEFINE_ACTOR(philosopher_actor, (start));
    int round = 0;

    nil::actor::future<> start() {
        auto arbitrator = ultramarine::get<arbitrator_actor>(0);
        return nil::actor::do_until([this] { return round >= RoundLen; }, [this, arbitrator] {
            return arbitrator->hungry(this->key).then([this, arbitrator](bool allowed) {
                if (allowed) {
                    ++round;
                    return arbitrator.tell(arbitrator_actor::message::done(), key);
                }
                ++failed_attempts;
                return nil::actor::make_ready_future();
            });
        });
    }
};

nil::actor::future<> dinning_philosophers() {
    failed_attempts = 0;
    return philosopher_actor::clear_directory().then([] {
        return arbitrator_actor::clear_directory().then([] {
            return nil::actor::parallel_for_each(boost::irange(0UL, PhilosopherLen), [](int philo) {
                return ultramarine::get<philosopher_actor>(philo)->start();
            }).then([] {
                nil::actor::print("performed a total of %d failed attempt\n", failed_attempts.load());
            });
        });
    });
}

int main(int ac, char **av) {
    return ultramarine::benchmark::run(ac, av, {
            ULTRAMARINE_BENCH(dinning_philosophers),
    }, 10);
}
