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

// Demonstration of file_input_stream.  Don't expect stellar performance
// since no read-ahead or caching is done yet.

#include <nil/actor/core/fstream.hh>
#include <nil/actor/core/core.hh>
#include <nil/actor/core/app-template.hh>
#include <nil/actor/core/shared_ptr.hh>
#include <fmt/printf.h>
#include <algorithm>
#include <iostream>

using namespace nil::actor;

struct reader {
public:
    reader(file f) : is(make_file_input_stream(std::move(f), file_input_stream_options {1 << 16, 1})) {
    }

    input_stream<char> is;
    size_t count = 0;

    // for input_stream::consume():
    using unconsumed_remainder = std::optional<temporary_buffer<char>>;
    future<unconsumed_remainder> operator()(temporary_buffer<char> data) {
        if (data.empty()) {
            return make_ready_future<unconsumed_remainder>(std::move(data));
        } else {
            count += std::count(data.begin(), data.end(), '\n');
            // FIXME: last line without \n?
            return make_ready_future<unconsumed_remainder>();
        }
    }
};

int main(int ac, char **av) {
    app_template app;
    namespace bpo = boost::program_options;
    app.add_positional_options({
        {"file", bpo::value<std::string>(), "File to process", 1},
    });
    return app.run(ac, av, [&app] {
        auto fname = app.configuration()["file"].as<std::string>();
        return open_file_dma(fname, open_flags::ro)
            .then([](file f) {
                auto r = make_shared<reader>(std::move(f));
                return r->is.consume(*r).then([r] {
                    fmt::print("{:d} lines\n", r->count);
                    return r->is.close().then([r] {});
                });
            })
            .then_wrapped([](future<> f) -> future<int> {
                try {
                    f.get();
                    return make_ready_future<int>(0);
                } catch (std::exception &ex) {
                    std::cout << ex.what() << "\n";
                    return make_ready_future<int>(1);
                } catch (...) {
                    std::cout << "unknown exception\n";
                    return make_ready_future<int>(1);
                }
            });
    });
}

