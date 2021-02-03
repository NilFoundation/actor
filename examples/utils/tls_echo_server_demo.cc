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

#include <cmath>
#include <nil/actor/core/reactor.hh>
#include <nil/actor/core/app-template.hh>
#include <nil/actor/core/sleep.hh>
#include <nil/actor/net/dns.hh>
#include "tls_echo_server.hh"

using namespace nil::actor;
namespace bpo = boost::program_options;

int main(int ac, char **av) {
    app_template app;
    app.add_options()("port", bpo::value<uint16_t>()->default_value(10000), "Server port")(
        "address", bpo::value<std::string>()->default_value("127.0.0.1"),
        "Server address")("cert,c", bpo::value<std::string>()->required(),
                          "Server certificate file")("key,k", bpo::value<std::string>()->required(), "Certificate key")(
        "verbose,v", bpo::value<bool>()->default_value(false)->implicit_value(true), "Verbose");
    return app.run_deprecated(ac, av, [&] {
        auto &&config = app.configuration();
        uint16_t port = config["port"].as<uint16_t>();
        auto crt = config["cert"].as<std::string>();
        auto key = config["key"].as<std::string>();
        auto addr = config["address"].as<std::string>();
        auto verbose = config["verbose"].as<bool>();

        std::cout << "Starting..." << std::endl;
        return net::dns::resolve_name(addr).then([=](net::inet_address a) {
            ipv4_addr ia(a, port);

            auto server = ::make_shared<nil::actor::sharded<echoserver>>();
            return server->start(verbose)
                .then([=]() {
                    return server->invoke_on_all(&echoserver::listen, socket_address(ia), sstring(crt), sstring(key),
                                                 tls::client_auth::NONE);
                })
                .handle_exception([=](auto e) {
                    std::cerr << "Error: " << e << std::endl;
                    engine().exit(1);
                })
                .then([=] {
                    std::cout << "TLS echo server running at " << addr << ":" << port << std::endl;
                    engine().at_exit([server] { return server->stop().finally([server] {}); });
                });
        });
    });
}
