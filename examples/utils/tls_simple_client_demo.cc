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

#include <nil/actor/core/shared_ptr.hh>
#include <nil/actor/core/reactor.hh>
#include <nil/actor/core/app-template.hh>
#include <nil/actor/core/sleep.hh>
#include <nil/actor/core/loop.hh>
#include <nil/actor/network/dns.hh>
#include "tls_echo_server.hh"

using namespace nil::actor;
namespace bpo = boost::program_options;

int main(int ac, char **av) {
    app_template app;
    app.add_options()("port", bpo::value<uint16_t>()->default_value(10000), "Remote port")(
        "address", bpo::value<std::string>()->default_value("127.0.0.1"), "Remote address")(
        "trust,t", bpo::value<std::string>(), "Trust store")("msg,m", bpo::value<std::string>(), "Message to send")(
        "bytes,b", bpo::value<size_t>()->default_value(512), "Use random bytes of length as message")(
        "iterations,i", bpo::value<size_t>()->default_value(1), "Repeat X times")(
        "read-response,r", bpo::value<bool>()->default_value(true)->implicit_value(true), "Read echoed message")(
        "verbose,v", bpo::value<bool>()->default_value(false)->implicit_value(true),
        "Verbose operation")("check-name,c", bpo::value<bool>()->default_value(false)->implicit_value(true),
                             "Check server name")("server-name,s", bpo::value<std::string>(), "Expected server name");

    return app.run_deprecated(ac, av, [&] {
        auto &&config = app.configuration();
        uint16_t port = config["port"].as<uint16_t>();
        auto addr = config["address"].as<std::string>();
        auto n = config["bytes"].as<size_t>();
        auto i = config["iterations"].as<size_t>();
        auto do_read = config["read-response"].as<bool>();
        auto verbose = config["verbose"].as<bool>();
        auto check = config["check-name"].as<bool>();

        std::cout << "Starting..." << std::endl;

        auto certs = ::make_shared<tls::certificate_credentials>();
        auto f = make_ready_future();

        if (config.count("trust")) {
            f = certs->set_x509_trust_file(config["trust"].as<std::string>(), tls::x509_crt_format::PEM);
        }

        nil::actor::shared_ptr<sstring> msg;

        if (config.count("msg")) {
            msg = nil::actor::make_shared<sstring>(config["msg"].as<std::string>());
        } else {
            msg = nil::actor::make_shared<sstring>(uninitialized_string(n));
            for (size_t i = 0; i < n; ++i) {
                (*msg)[i] = '0' + char(::rand() % 30);
            }
        }

        sstring server_name;
        if (config.count("server-name")) {
            server_name = config["server-name"].as<std::string>();
        }
        if (verbose) {
            std::cout << "Msg (" << msg->size() << "B):" << std::endl << *msg << std::endl;
        }
        return f
            .then([=]() {
                return net::dns::get_host_by_name(addr)
                    .then([=](net::hostent e) {
                        ipv4_addr ia(e.addr_list.front(), port);

                        sstring name;
                        if (check) {
                            name = server_name.empty() ? e.names.front() : server_name;
                        }
                        return tls::connect(certs, ia, name).then([=](::connected_socket s) {
                            auto strms = ::make_lw_shared<streams>(std::move(s));
                            auto range = boost::irange(size_t(0), i);
                            return do_for_each(range,
                                               [=](auto) {
                                                   auto f = strms->out.write(*msg);
                                                   if (!do_read) {
                                                       return strms->out.close().then(
                                                           [f = std::move(f)]() mutable { return std::move(f); });
                                                   }
                                                   return f.then([=]() {
                                                       return strms->out.flush().then([=] {
                                                           return strms->in.read_exactly(msg->size())
                                                               .then([=](temporary_buffer<char> buf) {
                                                                   sstring tmp(buf.begin(), buf.end());
                                                                   if (tmp != *msg) {
                                                                       std::cerr << "Got garbled message!" << std::endl;
                                                                       if (verbose) {
                                                                           std::cout << "Got (" << tmp.size()
                                                                                     << ") :" << std::endl
                                                                                     << tmp << std::endl;
                                                                       }
                                                                       throw std::runtime_error("Got garbled message!");
                                                                   }
                                                               });
                                                       });
                                                   });
                                               })
                                .then([strms, do_read] { return do_read ? strms->out.close() : make_ready_future<>(); })
                                .finally([strms] { return strms->in.close(); });
                        });
                    })
                    .handle_exception([](auto ep) { std::cerr << "Error: " << ep << std::endl; });
            })
            .finally([] { engine().exit(0); });
    });
}
