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
#include <nil/actor/core/seastar.hh>
#include <nil/actor/core/timer.hh>
#include <nil/actor/net/api.hh>

#include <chrono>
#include <iostream>

using namespace nil::actor;
using namespace net;
using namespace std::chrono_literals;

class client {
private:
    udp_channel _chan;
    uint64_t n_sent {};
    uint64_t n_received {};
    uint64_t n_failed {};
    timer<> _stats_timer;

public:
    void start(ipv4_addr server_addr) {
        std::cout << "Sending to " << server_addr << std::endl;

        _chan = make_udp_channel();

        _stats_timer.set_callback([this] {
            std::cout << "Out: " << n_sent << " pps, \t";
            std::cout << "Err: " << n_failed << " pps, \t";
            std::cout << "In: " << n_received << " pps" << std::endl;
            n_sent = 0;
            n_received = 0;
            n_failed = 0;
        });
        _stats_timer.arm_periodic(1s);

        // Run sender in background.
        (void)keep_doing([this, server_addr] {
            return _chan.send(server_addr, "hello!\n").then_wrapped([this](auto &&f) {
                try {
                    f.get();
                    n_sent++;
                } catch (...) {
                    n_failed++;
                }
            });
        });

        // Run receiver in background.
        (void)keep_doing([this] { return _chan.receive().then([this](auto) { n_received++; }); });
    }
};

namespace bpo = boost::program_options;

int main(int ac, char **av) {
    client _client;
    app_template app;
    app.add_options()("server", bpo::value<std::string>(), "Server address");
    return app.run_deprecated(ac, av, [&_client, &app] {
        auto &&config = app.configuration();
        _client.start(config["server"].as<std::string>());
    });
}
