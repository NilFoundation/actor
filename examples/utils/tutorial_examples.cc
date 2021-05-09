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

#include <nil/actor/core/core.hh>
#include <nil/actor/core/reactor.hh>
#include <nil/actor/core/future-util.hh>
#include <nil/actor/network/api.hh>

nil::actor::future<> service_loop() {
    return nil::actor::do_with(nil::actor::listen(nil::actor::make_ipv4_address({1234})), [](auto &listener) {
        return nil::actor::keep_doing([&listener]() {
            return listener.accept().then([](nil::actor::accept_result res) {
                std::cout << "Accepted connection from " << res.remote_address << "\n";
            });
        });
    });
}

const char *canned_response = "Actor is the future!\n";

nil::actor::future<> service_loop_2() {
    nil::actor::listen_options lo;
    lo.reuse_address = true;
    return nil::actor::do_with(nil::actor::listen(nil::actor::make_ipv4_address({1234}), lo), [](auto &listener) {
        return nil::actor::keep_doing([&listener]() {
            return listener.accept().then([](nil::actor::accept_result res) {
                auto s = std::move(res.connection);
                auto out = s.output();
                return nil::actor::do_with(std::move(s), std::move(out), [](auto &s, auto &out) {
                    return out.write(canned_response).then([&out] { return out.close(); });
                });
            });
        });
    });
}

nil::actor::future<> handle_connection_3(nil::actor::connected_socket s, nil::actor::socket_address a) {
    auto out = s.output();
    auto in = s.input();
    return do_with(std::move(s), std::move(out), std::move(in), [](auto &s, auto &out, auto &in) {
        return nil::actor::repeat([&out, &in] {
                   return in.read().then([&out](auto buf) {
                       if (buf) {
                           return out.write(std::move(buf)).then([&out] { return out.flush(); }).then([] {
                               return nil::actor::stop_iteration::no;
                           });
                       } else {
                           return nil::actor::make_ready_future<nil::actor::stop_iteration>(
                               nil::actor::stop_iteration::yes);
                       }
                   });
               })
            .then([&out] { return out.close(); });
    });
}

nil::actor::future<> service_loop_3() {
    nil::actor::listen_options lo;
    lo.reuse_address = true;
    return nil::actor::do_with(nil::actor::listen(nil::actor::make_ipv4_address({1234}), lo), [](auto &listener) {
        return nil::actor::keep_doing([&listener]() {
            return listener.accept().then([](nil::actor::accept_result res) {
                // Note we ignore, not return, the future returned by
                // handle_connection(), so we do not wait for one
                // connection to be handled before accepting the next one.
                (void)handle_connection_3(std::move(res.connection), std::move(res.remote_address))
                    .handle_exception(
                        [](std::exception_ptr ep) { fmt::print(stderr, "Could not handle connection: {}\n", ep); });
            });
        });
    });
}

#include <nil/actor/core/app-template.hh>

int main(int ac, char **av) {
    nil::actor::app_template app;
    return app.run(ac, av, [] {
        std::cout << "This is the tutorial examples demo.  It is not running anything but rather makes sure the "
                     "tutorial examples compile"
                  << std::endl;
        return nil::actor::make_ready_future<>();
    });
}
