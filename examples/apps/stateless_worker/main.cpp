//---------------------------------------------------------------------------//
// Copyright (c) 2018-2021 Mikhail Komarov <nemo@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#include <nil/actor/core/app-template.hh>
#include <nil/actor/core/print.hh>
#include <nil/actor/core/sleep.hh>
#include <nil/actor/core/future-util.hh>

#include <nil/actor/actor.hpp>
#include <nil/actor/actor_ref.hpp>

class worker : public nil::actor::actor<worker>,
               public nil::actor::local_actor<worker, 3>,
               public nil::actor::non_reentrant_actor<worker> {
    nil::actor::future<> say_hello() const {
        nil::actor::print("Hello, World; from simple_actor %d (%zu bytes) located on core %u.\n", key, sizeof(worker),
                          nil::actor::engine().cpu_id());

        // Simulate long-running job
        return nil::actor::sleep(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(500)));
    }

    ACTOR_DEFINE_ACTOR(worker, (say_hello));
};

int main(int ac, char **av) {
    nil::actor::app_template app;

    fmt::print("actor_ref size: {}\n", sizeof(nil::actor::actor_ref<worker>));
    fmt::print(" -- local_actor_ref size: {}\n", sizeof(nil::actor::impl::collocated_actor_ref<worker>));

    fmt::print("actor size: {}\n", sizeof(worker));
    fmt::print(" -- local_actor attribute size: {}\n", sizeof(nil::actor::local_actor<worker, 3>));
    fmt::print(" -- non_reentrant_actor attribute size: {}\n", sizeof(nil::actor::non_reentrant_actor<worker>));
    fmt::print(" -- key size: {}\n", sizeof(worker::KeyType));

    return app.run(ac, av, [] {
        return nil::actor::when_all(nil::actor::get<worker>(0)->say_hello(),
                                    nil::actor::get<worker>(0)->say_hello(),
                                    nil::actor::get<worker>(0)->say_hello(),
                                    nil::actor::get<worker>(0)->say_hello(),
                                    nil::actor::get<worker>(0)->say_hello(),
                                    nil::actor::get<worker>(0)->say_hello())
            .discard_result();
    });
}
