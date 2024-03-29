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

#include <nil/actor/core/reactor.hh>
#include <nil/actor/core/app-template.hh>
#include <nil/actor/core/temporary_buffer.hh>
#include <nil/actor/core/distributed.hh>
#include <nil/actor/core/print.hh>

#include <vector>
#include <iostream>

using namespace nil::actor;

static std::string str_ping {"ping"};
static std::string str_txtx {"txtx"};
static std::string str_rxrx {"rxrx"};
static std::string str_pong {"pong"};
static std::string str_unknow {"unknow cmd"};
static int tx_msg_total_size = 100 * 1024 * 1024;
static int tx_msg_size = 4 * 1024;
static int tx_msg_nr = tx_msg_total_size / tx_msg_size;
static int rx_msg_size = 4 * 1024;
static std::string str_txbuf(tx_msg_size, 'X');
static bool enable_tcp = false;
static bool enable_sctp = false;

class tcp_server {
    std::vector<server_socket> _tcp_listeners;
    std::vector<server_socket> _sctp_listeners;

public:
    future<> listen(ipv4_addr addr) {
        if (enable_tcp) {
            listen_options lo;
            lo.proto = transport::TCP;
            lo.reuse_address = true;
            _tcp_listeners.push_back(nil::actor::listen(make_ipv4_address(addr), lo));
            do_accepts(_tcp_listeners);
        }

        if (enable_sctp) {
            listen_options lo;
            lo.proto = transport::SCTP;
            lo.reuse_address = true;
            _sctp_listeners.push_back(nil::actor::listen(make_ipv4_address(addr), lo));
            do_accepts(_sctp_listeners);
        }
        return make_ready_future<>();
    }

    // FIXME: We should properly tear down the service here.
    future<> stop() {
        return make_ready_future<>();
    }

    void do_accepts(std::vector<server_socket> &listeners) {
        int which = listeners.size() - 1;
        // Accept in the background.
        (void)listeners[which]
            .accept()
            .then([this, &listeners](accept_result ar) mutable {
                connected_socket fd = std::move(ar.connection);
                socket_address addr = std::move(ar.remote_address);
                auto conn = new connection(*this, std::move(fd), addr);
                (void)conn->process().then_wrapped([conn](auto &&f) {
                    delete conn;
                    try {
                        f.get();
                    } catch (std::exception &ex) {
                        std::cout << "request error " << ex.what() << "\n";
                    }
                });
                do_accepts(listeners);
            })
            .then_wrapped([](auto &&f) {
                try {
                    f.get();
                } catch (std::exception &ex) {
                    std::cout << "accept failed: " << ex.what() << "\n";
                }
            });
    }
    class connection {
        connected_socket _fd;
        input_stream<char> _read_buf;
        output_stream<char> _write_buf;

    public:
        connection(tcp_server &server, connected_socket &&fd, socket_address addr) :
            _fd(std::move(fd)), _read_buf(_fd.input()), _write_buf(_fd.output()) {
        }
        future<> process() {
            return read();
        }
        future<> read() {
            if (_read_buf.eof()) {
                return make_ready_future();
            }
            // Expect 4 bytes cmd from client
            size_t n = 4;
            return _read_buf.read_exactly(n).then([this](temporary_buffer<char> buf) {
                if (buf.size() == 0) {
                    return make_ready_future();
                }
                auto cmd = std::string(buf.get(), buf.size());
                // pingpong test
                if (cmd == str_ping) {
                    return _write_buf.write(str_pong).then([this] { return _write_buf.flush(); }).then([this] {
                        return this->read();
                    });
                    // server tx test
                } else if (cmd == str_txtx) {
                    return tx_test();
                    // server tx test
                } else if (cmd == str_rxrx) {
                    return rx_test();
                    // unknow test
                } else {
                    return _write_buf.write(str_unknow).then([this] { return _write_buf.flush(); }).then([] {
                        return make_ready_future();
                    });
                }
            });
        }
        future<> do_write(int end) {
            if (end == 0) {
                return make_ready_future<>();
            }
            return _write_buf.write(str_txbuf).then([this] { return _write_buf.flush(); }).then([this, end] {
                return do_write(end - 1);
            });
        }
        future<> tx_test() {
            return do_write(tx_msg_nr).then([this] { return _write_buf.close(); }).then([] {
                return make_ready_future<>();
            });
        }
        future<> do_read() {
            return _read_buf.read_exactly(rx_msg_size).then([this](temporary_buffer<char> buf) {
                if (buf.size() == 0) {
                    return make_ready_future();
                } else {
                    return do_read();
                }
            });
        }
        future<> rx_test() {
            return do_read().then([] { return make_ready_future<>(); });
        }
    };
};

namespace bpo = boost::program_options;

int main(int ac, char **av) {
    app_template app;
    app.add_options()("port", bpo::value<uint16_t>()->default_value(10000),
                      "TCP server port")("tcp", bpo::value<std::string>()->default_value("yes"), "tcp listen")(
        "sctp", bpo::value<std::string>()->default_value("no"), "sctp listen");
    return app.run_deprecated(ac, av, [&] {
        auto &&config = app.configuration();
        uint16_t port = config["port"].as<uint16_t>();
        enable_tcp = config["tcp"].as<std::string>() == "yes";
        enable_sctp = config["sctp"].as<std::string>() == "yes";
        if (!enable_tcp && !enable_sctp) {
            fprint(std::cerr, "Error: no protocols enabled. Use \"--tcp yes\" and/or \"--sctp yes\" to enable\n");
            return engine().exit(1);
        }
        auto server = new distributed<tcp_server>;
        (void)server->start()
            .then([server = std::move(server), port]() mutable {
                engine().at_exit([server] { return server->stop(); });
                // Start listening in the background.
                (void)server->invoke_on_all(&tcp_server::listen, ipv4_addr {port});
            })
            .then([port] { std::cout << "Actor TCP server listening on port " << port << " ...\n"; });
    });
}
