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

#include <algorithm>
#include <iostream>

#include <nil/actor/core/app-template.hh>
#include <nil/actor/core/file.hh>
#include <nil/actor/core/reactor.hh>
#include <nil/actor/core/seastar.hh>
#include <nil/actor/core/semaphore.hh>

using namespace nil::actor;

namespace bpo = boost::program_options;

struct file_test {
    file_test(file &&f) : f(std::move(f)) {
    }
    file f;
    semaphore sem = {0};
};

int main(int ac, char **av) {
    app_template app;
    app.add_options()("dev", bpo::value<std::string>(), "e.g. --dev /dev/sdb");

    return app.run_deprecated(ac, av, [&app] {
        static constexpr auto max = 10000;
        auto &&config = app.configuration();
        auto filepath = config["dev"].as<std::string>();

        return open_file_dma(filepath, open_flags::rw | open_flags::create).then([](file f) {
            auto ft = new file_test {std::move(f)};

            // Discard asynchronously, siganl when done.
            (void)ft->f.stat().then([ft](struct stat st) mutable {
                assert(S_ISBLK(st.st_mode));
                auto offset = 0;
                auto length = max * 4096;
                return ft->f.discard(offset, length).then([ft]() mutable { ft->sem.signal(); });
            });

            // Wait and exit.
            (void)ft->sem.wait().then([ft]() mutable { return ft->f.flush(); }).then([ft]() mutable {
                std::cout << "done\n";
                delete ft;
                engine().exit(0);
            });
        });
    });
}
