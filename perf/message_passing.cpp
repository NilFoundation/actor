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
#include <ultramarine/message_deduplicate.hpp>
#include <ultramarine/utility.hpp>
#include "benchmark_utility.hpp"

class counter_actor : public ultramarine::actor<counter_actor> {
public:
    volatile int counter = 0;

    nil::actor::future<> increase_counter_future() {
        counter++;
        return nil::actor::make_ready_future();
    }

    nil::actor::future<int> noop(int i) const {
        return nil::actor::make_ready_future<int>(i);
    }

    void increase_counter_void() {
        counter++;
    }

    nil::actor::future<int> get_counter_future() const {
        return nil::actor::make_ready_future<int>(counter);
    }

    int get_counter_int() const {
        return counter;
    }

    nil::actor::future<int> accumulate_future(std::vector<int> pack) const {
        return nil::actor::make_ready_future<int>(std::accumulate(std::begin(pack), std::end(pack), 0));
    }

    int accumulate_value(std::vector<int> pack) const {
        return std::accumulate(std::begin(pack), std::end(pack), 0);
    }

ULTRAMARINE_DEFINE_ACTOR(counter_actor,
                         (increase_counter_future)(increase_counter_void)
                                 (get_counter_future)(get_counter_int)
                                 (accumulate_future)(accumulate_value)(noop));
};

/*
 * PLAIN OBJECT
 */

auto plain_object_void_future() {
    int *counter = new int(0);

    auto counterActor = new counter_actor(0);
    return nil::actor::do_until([counterActor, counter] {
        return *counter >= 10000;
    }, [counterActor, counter] {
        ++*counter;
        return counterActor->increase_counter_future();
    });
}

auto plain_object_void() {
    int *counter = new int(0);

    auto counterActor = new counter_actor(0);
    return nil::actor::do_until([counterActor, counter] {
        return *counter >= 10000;
    }, [counterActor, counter] {
        ++*counter;
        counterActor->increase_counter_void();
        return nil::actor::make_ready_future();
    });
}

auto plain_object_int_future() {
    int *counter = new int(0);

    auto counterActor = new counter_actor(0);
    return nil::actor::do_until([counterActor, counter] {
        return *counter >= 10000;
    }, [counterActor, counter] {
        ++*counter;
        return counterActor->get_counter_future().discard_result();
    });
}

auto plain_object_int() {
    int *counter = new int(0);

    auto counterActor = new counter_actor(0);
    return nil::actor::do_until([counterActor, counter] {
        return *counter >= 10000;
    }, [counterActor, counter] {
        ++*counter;
        counterActor->get_counter_int();
        return nil::actor::make_ready_future();
    });
}

auto plain_object_future_args() {
    int *counter = new int(0);

    auto counterActor = new counter_actor(0);
    return nil::actor::do_until([counterActor, counter] {
        return *counter >= 10000;
    }, [counterActor, counter] {
        ++*counter;
        return counterActor->accumulate_future({10, 12, 30, *counter}).discard_result();
    });
}

auto plain_object_int_args() {
    int *counter = new int(0);

    auto counterActor = new counter_actor(0);
    return nil::actor::do_until([counterActor, counter] {
        return *counter >= 10000;
    }, [counterActor, counter] {
        ++*counter;
        counterActor->accumulate_value(std::vector<int>{10, 12, 30, *counter});
        return nil::actor::make_ready_future();
    });
}

/*
 * LOCAL ACTOR
 */

auto local_actor_void_future() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(0);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter]() mutable {
        ++*counter;
        return counterActor.tell(counter_actor::message::increase_counter_future());
    });

}

auto local_actor_void() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(0);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter]() mutable {
        ++*counter;
        return counterActor.tell(counter_actor::message::increase_counter_void());
    });
}

auto local_actor_int_future() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(0);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter]() {
        ++*counter;
        return counterActor.tell(counter_actor::message::get_counter_future()).discard_result();
    });
}

auto local_actor_int() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(0);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter]() {
        ++*counter;
        return counterActor.tell(counter_actor::message::get_counter_int()).discard_result();
    });
}

auto local_actor_future_args() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(0);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter]() {
        ++*counter;
        return counterActor.tell(counter_actor::message::accumulate_future(),
                                 std::vector<int>{10, 12, 30, *counter}).discard_result();
    });
}

auto local_actor_int_args() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(0);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter]() {
        ++*counter;
        return counterActor.tell(counter_actor::message::accumulate_value(),
                                 std::vector<int>{10, 12, 30, *counter}).discard_result();
    });
}

/*
 * LOCAL ACTOR (DEDUPLICATED)
 */

auto local_actor_deduplicated_void_future() {
    auto counterActor = ultramarine::get<counter_actor>(0);
    return ultramarine::deduplicate(counterActor, counter_actor::message::increase_counter_future(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d();
        }
    });
}

auto local_actor_deduplicated_void() {
    auto counterActor = ultramarine::get<counter_actor>(0);
    return ultramarine::deduplicate(counterActor, counter_actor::message::increase_counter_void(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d();
        }
    });
}

auto local_actor_deduplicated_int_future() {
    auto counterActor = ultramarine::get<counter_actor>(0);
    return ultramarine::deduplicate(counterActor, counter_actor::message::get_counter_future(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d();
        }
    }).discard_result();
}

auto local_actor_deduplicated_int() {
    auto counterActor = ultramarine::get<counter_actor>(0);
    return ultramarine::deduplicate(counterActor, counter_actor::message::get_counter_int(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d();
        }
    }).discard_result();
}

auto local_actor_deduplicated_future_args() {
    auto counterActor = ultramarine::get<counter_actor>(0);
    return ultramarine::deduplicate(counterActor, counter_actor::message::accumulate_future(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d(std::vector<int>{10, 12, 30, j});
        }
    }).discard_result();
}

auto local_actor_deduplicated_int_args() {
    auto counterActor = ultramarine::get<counter_actor>(0);
    return ultramarine::deduplicate(counterActor, counter_actor::message::accumulate_value(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d(std::vector<int>{10, 12, 30, j});
        }
    }).discard_result();
}

/*
 * COLLOCATED ACTOR
 */

auto collocated_actor_void_future() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(1);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter]() mutable {
        ++*counter;
        return counterActor.tell(counter_actor::message::increase_counter_future());
    });
}

auto collocated_actor_void() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(1);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter]() mutable {
        ++*counter;
        return counterActor.tell(counter_actor::message::increase_counter_void());
    });
}

auto collocated_actor_int_future() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(1);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter] {
        ++*counter;
        return counterActor.tell(counter_actor::message::get_counter_future()).discard_result();
    });
}

auto collocated_actor_int() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(1);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter] {
        ++*counter;
        return counterActor.tell(counter_actor::message::get_counter_int()).discard_result();
    });
}

auto collocated_actor_future_args() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(1);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter] {
        ++*counter;
        return counterActor.tell(counter_actor::message::accumulate_future(),
                                 std::vector<int>{10, 12, 30, *counter}).discard_result();
    });
}

auto collocated_actor_int_args() {
    int *counter = new int(0);

    auto counterActor = ultramarine::get<counter_actor>(1);
    return nil::actor::do_until([counter] {
        return *counter >= 10000;
    }, [counterActor, counter] {
        ++*counter;
        return counterActor.tell(counter_actor::message::accumulate_value(),
                                 std::vector<int>{10, 12, 30, *counter}).discard_result();
    });
}

/*
 * COLLOCATED ACTOR (DEDUPLICATED)
 */

auto collocated_actor_deduplicated_void_future() {
    auto counterActor = ultramarine::get<counter_actor>(1);
    return ultramarine::deduplicate(counterActor, counter_actor::message::increase_counter_future(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d();
        }
    });
}

auto collocated_actor_deduplicated_void() {
    auto counterActor = ultramarine::get<counter_actor>(1);
    return ultramarine::deduplicate(counterActor, counter_actor::message::increase_counter_void(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d();
        }
    });
}

auto collocated_actor_deduplicated_int_future() {
    auto counterActor = ultramarine::get<counter_actor>(1);
    return ultramarine::deduplicate(counterActor, counter_actor::message::get_counter_future(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d();
        }
    }).discard_result();
}

auto collocated_actor_deduplicated_int() {
    auto counterActor = ultramarine::get<counter_actor>(1);
    return ultramarine::deduplicate(counterActor, counter_actor::message::get_counter_int(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d();
        }
    }).discard_result();
}

auto collocated_actor_deduplicated_future_args() {
    auto counterActor = ultramarine::get<counter_actor>(1);
    return ultramarine::deduplicate(counterActor, counter_actor::message::accumulate_future(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d(std::vector<int>{10, 12, 30, j});
        }
    }).discard_result();
}

auto collocated_actor_deduplicated_int_args() {
    auto counterActor = ultramarine::get<counter_actor>(1);
    return ultramarine::deduplicate(counterActor, counter_actor::message::accumulate_value(), [](auto &d) {
        for (int j = 0; j < 10000; ++j) {
            d(std::vector<int>{10, 12, 30, j});
        }
    }).discard_result();
}

int main(int ac, char **av) {
    return ultramarine::benchmark::run(ac, av, {
            ULTRAMARINE_BENCH(plain_object_void_future),
            ULTRAMARINE_BENCH(local_actor_void_future),
            ULTRAMARINE_BENCH(local_actor_deduplicated_void_future),
            ULTRAMARINE_BENCH(collocated_actor_void_future),
            ULTRAMARINE_BENCH(collocated_actor_deduplicated_void_future),
            ULTRAMARINE_BENCH(plain_object_void),
            ULTRAMARINE_BENCH(local_actor_void),
            ULTRAMARINE_BENCH(local_actor_deduplicated_void),
            ULTRAMARINE_BENCH(collocated_actor_void),
            ULTRAMARINE_BENCH(collocated_actor_deduplicated_void),
            ULTRAMARINE_BENCH(plain_object_int_future),
            ULTRAMARINE_BENCH(local_actor_int_future),
            ULTRAMARINE_BENCH(local_actor_deduplicated_int_future),
            ULTRAMARINE_BENCH(collocated_actor_int_future),
            ULTRAMARINE_BENCH(collocated_actor_deduplicated_int_future),
            ULTRAMARINE_BENCH(plain_object_int),
            ULTRAMARINE_BENCH(local_actor_int),
            ULTRAMARINE_BENCH(local_actor_deduplicated_int),
            ULTRAMARINE_BENCH(collocated_actor_int),
            ULTRAMARINE_BENCH(collocated_actor_deduplicated_int),
            ULTRAMARINE_BENCH(plain_object_future_args),
            ULTRAMARINE_BENCH(local_actor_future_args),
            ULTRAMARINE_BENCH(local_actor_deduplicated_future_args),
            ULTRAMARINE_BENCH(collocated_actor_future_args),
            ULTRAMARINE_BENCH(collocated_actor_deduplicated_future_args),
            ULTRAMARINE_BENCH(plain_object_int_args),
            ULTRAMARINE_BENCH(local_actor_int_args),
            ULTRAMARINE_BENCH(local_actor_deduplicated_int_args),
            ULTRAMARINE_BENCH(collocated_actor_int_args),
            ULTRAMARINE_BENCH(collocated_actor_deduplicated_int_args)
    }, 1000);
}
