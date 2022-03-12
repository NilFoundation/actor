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

#include <nil/actor/http/httpd.hh>
#include <nil/actor/http/handlers.hh>
#include <nil/actor/http/function_handlers.hh>
#include <nil/actor/http/file_handler.hh>
#include <nil/actor/core/core.hh>
#include <nil/actor/core/reactor.hh>
#include "demo.json.hh"
#include <nil/actor/http/api_docs.hh>
#include <nil/actor/core/thread.hh>
#include <nil/actor/core/prometheus.hh>
#include <nil/actor/core/print.hh>
#include <nil/actor/network/inet_address.hh>
#include "../lib/stop_signal.hh"

namespace bpo = boost::program_options;

using namespace nil::actor;
using namespace httpd;

class handl : public httpd::handler_base {
public:
    virtual future<std::unique_ptr<reply>> handle(const sstring &path, std::unique_ptr<request> req,
                                                  std::unique_ptr<reply> rep) {
        rep->_content = "hello";
        rep->done("html");
        return make_ready_future<std::unique_ptr<reply>>(std::move(rep));
    }
};

void set_routes(routes &r) {
    function_handler *h1 = new function_handler([](const_req req) { return "hello"; });
    function_handler *h2 = new function_handler(
        [](std::unique_ptr<request> req) { return make_ready_future<json::json_return_type>("json-future"); });
    r.add(operation_type::GET, url("/"), h1);
    r.add(operation_type::GET, url("/jf"), h2);
    r.add(operation_type::GET, url("/file").remainder("path"), new directory_handler("/"));
    demo_json::hello_world.set(r, [](const_req req) {
        demo_json::my_object obj;
        obj.var1 = req.param.at("var1");
        obj.var2 = req.param.at("var2");
        demo_json::ns_hello_world::query_enum v =
            demo_json::ns_hello_world::str2query_enum(req.query_parameters.at("query_enum"));
        // This demonstrate enum conversion
        obj.enum_var = v;
        return obj;
    });
}

int main(int ac, char **av) {
    httpd::http_server_control prometheus_server;
    prometheus::config pctx;
    app_template app;

    app.add_options()("port", bpo::value<uint16_t>()->default_value(10000), "HTTP Server port");
    app.add_options()("prometheus_port", bpo::value<uint16_t>()->default_value(9180),
                      "Prometheus port. Set to zero in order to disable.");
    app.add_options()("prometheus_address", bpo::value<sstring>()->default_value("0.0.0.0"), "Prometheus address");
    app.add_options()("prometheus_prefix", bpo::value<sstring>()->default_value("seastar_httpd"),
                      "Prometheus metrics prefix");

    return app.run_deprecated(ac, av, [&] {
        return nil::actor::async([&] {
            actor_apps_lib::stop_signal stop_signal;
            auto &&config = app.configuration();
            httpd::http_server_control prometheus_server;

            uint16_t pport = config["prometheus_port"].as<uint16_t>();
            if (pport) {
                prometheus::config pctx;
                net::inet_address prom_addr(config["prometheus_address"].as<sstring>());

                pctx.metric_help = "nil::actor::httpd server statistics";
                pctx.prefix = config["prometheus_prefix"].as<sstring>();

                std::cout << "starting prometheus API server" << std::endl;
                prometheus_server.start("prometheus").get();

                prometheus::start(prometheus_server, pctx).get();

                prometheus_server.listen(socket_address {prom_addr, pport})
                    .handle_exception([prom_addr, pport](auto ep) {
                        std::cerr << nil::actor::format("Could not start Prometheus API server on {}:{}: {}\n",
                                                        prom_addr, pport, ep);
                        return make_exception_future<>(ep);
                    })
                    .get();
            }

            uint16_t port = config["port"].as<uint16_t>();
            auto server = new http_server_control();
            auto rb = make_shared<api_registry_builder>("apps/httpd/");
            server->start().get();
            server->set_routes(set_routes).get();
            server->set_routes([rb](routes &r) { rb->set_api_doc(r); }).get();
            server->set_routes([rb](routes &r) { rb->register_function(r, "demo", "hello world application"); }).get();
            server->listen(port).get();

            std::cout << "Actor HTTP server listening on port " << port << " ...\n";
            engine().at_exit([&prometheus_server, server, pport] {
                return [pport, &prometheus_server] {
                    if (pport) {
                        std::cout << "Stoppping Prometheus server" << std::endl;
                        return prometheus_server.stop();
                    }
                    return make_ready_future<>();
                }()
                           .finally([server] {
                               std::cout << "Stoppping HTTP server" << std::endl;
                               return server->stop();
                           });
            });

            stop_signal.wait().get();
        });
    });
}
