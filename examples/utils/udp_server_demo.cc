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

#include <nil/actor/core/reactor.hh>
#include <nil/actor/core/distributed.hh>
#include <nil/actor/core/app-template.hh>

using namespace nil::actor;
using namespace net;
using namespace std::chrono_literals;

class udp_server {
private:
    udp_channel _chan;
    timer<> _stats_timer;
    uint64_t _n_sent {};

public:
    void start(uint16_t port) {
        ipv4_addr listen_addr {port};
        _chan = make_udp_channel(listen_addr);

        _stats_timer.set_callback([this] {
            std::cout << "Out: " << _n_sent << " pps" << std::endl;
            _n_sent = 0;
        });
        _stats_timer.arm_periodic(1s);

        // Run server in background.
        (void)keep_doing([this] {
            return _chan.receive().then([this](udp_datagram dgram) {
                return _chan.send(dgram.get_src(), std::move(dgram.get_data())).then([this] { _n_sent++; });
            });
        });
    }
    // FIXME: we should properly tear down the service here.
    future<> stop() {
        return make_ready_future<>();
    }
};

namespace bpo = boost::program_options;

int main(int ac, char **av) {
    app_template app;
    app.add_options()("port", bpo::value<uint16_t>()->default_value(10000), "UDP server port");
    return app.run_deprecated(ac, av, [&] {
        auto &&config = app.configuration();
        uint16_t port = config["port"].as<uint16_t>();
        auto server = new distributed<udp_server>;
        // Run server in background.
        (void)server->start()
            .then([server = std::move(server), port]() mutable {
                engine().at_exit([server] { return server->stop(); });
                return server->invoke_on_all(&udp_server::start, port);
            })
            .then([port] { std::cout << "Seastar UDP server listening on port " << port << " ...\n"; });
    });
}
