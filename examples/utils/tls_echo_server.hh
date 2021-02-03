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

#pragma once

#include <nil/actor/core/sstring.hh>
#include <nil/actor/core/do_with.hh>
#include <nil/actor/core/sharded.hh>
#include <nil/actor/core/gate.hh>
#include <nil/actor/core/loop.hh>
#include <nil/actor/net/tls.hh>
#include <nil/actor/detail/log.hh>
#include <iostream>

using namespace nil::actor;

struct streams {
    connected_socket s;
    input_stream<char> in;
    output_stream<char> out;

    streams(connected_socket cs) : s(std::move(cs)), in(s.input()), out(s.output()) {
    }
};

class echoserver {
    server_socket _socket;
    shared_ptr<tls::server_credentials> _certs;
    nil::actor::gate _gate;
    bool _stopped = false;
    bool _verbose = false;

public:
    echoserver(bool verbose = false) :
        _certs(make_shared<tls::server_credentials>(make_shared<tls::dh_params>())), _verbose(verbose) {
    }

    future<> listen(socket_address addr, sstring crtfile, sstring keyfile,
                    tls::client_auth ca = tls::client_auth::NONE) {
        _certs->set_client_auth(ca);
        return _certs->set_x509_key_file(crtfile, keyfile, tls::x509_crt_format::PEM).then([this, addr] {
            ::listen_options opts;
            opts.reuse_address = true;

            _socket = tls::listen(_certs, addr, opts);

            // Listen in background.
            (void)repeat([this] {
                if (_stopped) {
                    return make_ready_future<stop_iteration>(stop_iteration::yes);
                }
                return with_gate(_gate, [this] {
                    return _socket.accept()
                        .then([this](accept_result ar) {
                            ::connected_socket s = std::move(ar.connection);
                            socket_address a = std::move(ar.remote_address);
                            if (_verbose) {
                                std::cout << "Got connection from " << a << std::endl;
                            }
                            auto strms = make_lw_shared<streams>(std::move(s));
                            return repeat([strms, this]() {
                                       return strms->in.read().then([this, strms](temporary_buffer<char> buf) {
                                           if (buf.empty()) {
                                               if (_verbose) {
                                                   std::cout << "EOM" << std::endl;
                                               }
                                               return make_ready_future<stop_iteration>(stop_iteration::yes);
                                           }
                                           sstring tmp(buf.begin(), buf.end());
                                           if (_verbose) {
                                               std::cout << "Read " << tmp.size() << "B" << std::endl;
                                           }
                                           return strms->out.write(tmp)
                                               .then([strms]() { return strms->out.flush(); })
                                               .then([] {
                                                   return make_ready_future<stop_iteration>(stop_iteration::no);
                                               });
                                       });
                                   })
                                .then([strms] { return strms->out.close(); })
                                .handle_exception([](auto ep) {})
                                .finally([this, strms] {
                                    if (_verbose) {
                                        std::cout << "Ending session" << std::endl;
                                    }
                                    return strms->in.close();
                                });
                        })
                        .handle_exception([this](auto ep) {
                            if (!_stopped) {
                                std::cerr << "Error: " << ep << std::endl;
                            }
                        })
                        .then([this] {
                            return make_ready_future<stop_iteration>(_stopped ? stop_iteration::yes :
                                                                                stop_iteration::no);
                        });
                });
            });
            return make_ready_future();
        });
    }

    future<> stop() {
        _stopped = true;
        _socket.abort_accept();
        return _gate.close();
    }
};
