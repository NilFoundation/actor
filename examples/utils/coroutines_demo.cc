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

#include <iostream>

#include <nil/actor/detail/std-compat.hh>

#ifndef ACTOR_COROUTINES_ENABLED

int main(int argc, char **argv) {
    std::cout << "coroutines not available\n";
    return 0;
}

#else

#include <nil/actor/core/app-template.hh>
#include <nil/actor/core/coroutine.hh>
#include <nil/actor/core/fstream.hh>
#include <nil/actor/core/sleep.hh>
#include <nil/actor/core/seastar.hh>
#include <nil/actor/core/loop.hh>

int main(int argc, char **argv) {
    nil::actor::app_template app;
    app.run(argc, argv, []() -> nil::actor::future<> {
        std::cout << "this is a completely useless program\nplease stand by...\n";
        auto f = nil::actor::parallel_for_each(std::vector<int> {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               [](int i) -> nil::actor::future<> {
                                                   co_await nil::actor::sleep(std::chrono::seconds(i));
                                                   std::cout << i << "\n";
                                               });

        auto file = co_await nil::actor::open_file_dma("useless_file.txt",
                                                       nil::actor::open_flags::create | nil::actor::open_flags::wo);
        auto out = co_await nil::actor::make_file_output_stream(file);
        nil::actor::sstring str = "nothing to see here, move along now\n";
        co_await out.write(str);
        co_await out.flush();
        co_await out.close();

        co_await std::move(f);
        std::cout << "done\n";
    });
}

#endif
