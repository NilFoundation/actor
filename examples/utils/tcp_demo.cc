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

#include <nil/actor/network/ip.hh>
#include <nil/actor/network/virtio.hh>
#include <nil/actor/network/tcp.hh>
#include <nil/actor/core/reactor.hh>
#include <fmt/printf.h>

using namespace nil::actor;
using namespace net;

struct tcp_test {
    ipv4 &inet;
    using tcp = net::tcp<ipv4_traits>;
    tcp::listener _listener;
    struct connection {
        tcp::connection tcp_conn;
        explicit connection(tcp::connection tc) : tcp_conn(std::move(tc)) {
        }
        void run() {
            // Read packets and echo back in the background.
            (void)tcp_conn.wait_for_data().then([this] {
                auto p = tcp_conn.read();
                if (!p.len()) {
                    tcp_conn.close_write();
                    return;
                }
                fmt::print("read {:d} bytes\n", p.len());
                (void)tcp_conn.send(std::move(p));
                run();
            });
        }
    };
    tcp_test(ipv4 &inet) : inet(inet), _listener(inet.get_tcp().listen(10000)) {
    }
    void run() {
        // Run all connections in the background.
        (void)_listener.accept().then([this](tcp::connection conn) {
            (new connection(std::move(conn)))->run();
            run();
        });
    }
};

int main(int ac, char **av) {
    boost::program_options::variables_map opts;
    opts.insert(std::make_pair("tap-device", boost::program_options::variable_value(std::string("tap0"), false)));

    auto vnet = create_virtio_net_device(opts);
    interface netif(std::move(vnet));
    ipv4 inet(&netif);
    inet.set_host_address(ipv4_address("192.168.122.2"));
    tcp_test tt(inet);
    (void)engine().when_started().then([&tt] { tt.run(); });
    engine().run();
}

