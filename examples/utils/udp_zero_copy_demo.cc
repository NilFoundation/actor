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

#include <nil/actor/core/core.hh>
#include <nil/actor/core/app-template.hh>
#include <nil/actor/core/scattered_message.hh>
#include <nil/actor/core/vector-data-sink.hh>
#include <nil/actor/core/shared_ptr.hh>
#include <nil/actor/core/units.hh>
#include <nil/actor/core/timer.hh>

#include <nil/actor/network/api.hh>

#include <random>
#include <iomanip>
#include <iostream>

using namespace nil::actor;
using namespace net;
using namespace std::chrono_literals;
namespace bpo = boost::program_options;

template<typename Duration>
typename Duration::rep to_seconds(Duration d) {
    return std::chrono::duration_cast<std::chrono::seconds>(d).count();
}

class server {
private:
    udp_channel _chan;
    timer<> _stats_timer;
    uint64_t _n_sent {};
    size_t _chunk_size;
    bool _copy;
    std::vector<packet> _packets;
    std::unique_ptr<output_stream<char>> _out;
    steady_clock_type::time_point _last;
    sstring _key;
    size_t _packet_size = 8 * KB;
    char *_mem;
    size_t _mem_size;
    std::mt19937 _rnd;
    std::random_device _randem_dev;
    std::uniform_int_distribution<size_t> _chunk_distribution;

private:
    char *next_chunk() {
        return _mem + _chunk_distribution(_rnd);
    }

public:
    server() : _rnd(std::random_device()()) {
    }
    future<> send(ipv4_addr dst, packet p) {
        return _chan.send(dst, std::move(p)).then([this] { _n_sent++; });
    }
    void start(int chunk_size, bool copy, size_t mem_size) {
        ipv4_addr listen_addr {10000};
        _chan = make_udp_channel(listen_addr);

        std::cout << "Listening on " << listen_addr << std::endl;

        _last = steady_clock_type::now();
        _stats_timer.set_callback([this] {
            auto now = steady_clock_type::now();
            std::cout << "Out: " << std::setprecision(2) << std::fixed << (double)_n_sent / to_seconds(now - _last)
                      << " pps" << std::endl;
            _last = now;
            _n_sent = 0;
        });
        _stats_timer.arm_periodic(1s);

        _chunk_size = chunk_size;
        _copy = copy;
        _key = sstring(new char[64], 64);

        _out = std::make_unique<output_stream<char>>(data_sink(std::make_unique<vector_data_sink>(_packets)),
                                                     _packet_size);

        _mem = new char[mem_size];
        _mem_size = mem_size;

        _chunk_distribution = std::uniform_int_distribution<size_t>(0, _mem_size - _chunk_size * 3);

        assert(3 * _chunk_size <= _packet_size);

        // Run sender in background.
        (void)keep_doing([this] {
            return _chan.receive().then([this](udp_datagram dgram) {
                auto chunk = next_chunk();
                lw_shared_ptr<sstring> item;
                if (_copy) {
                    _packets.clear();
                    // FIXME: future is discarded
                    (void)_out->write(chunk, _chunk_size);
                    chunk += _chunk_size;
                    (void)_out->write(chunk, _chunk_size);
                    chunk += _chunk_size;
                    (void)_out->write(chunk, _chunk_size);
                    (void)_out->flush();
                    assert(_packets.size() == 1);
                    return send(dgram.get_src(), std::move(_packets[0]));
                } else {
                    auto chunk = next_chunk();
                    scattered_message<char> msg;
                    msg.reserve(3);
                    msg.append_static(chunk, _chunk_size);
                    msg.append_static(chunk, _chunk_size);
                    msg.append_static(chunk, _chunk_size);
                    return send(dgram.get_src(), std::move(msg).release());
                }
            });
        });
    }
};

int main(int ac, char **av) {
    server s;
    app_template app;
    app.add_options()("chunk-size", bpo::value<int>()->default_value(1024),
                      "Chunk size")("mem-size", bpo::value<int>()->default_value(512),
                                    "Memory pool size in MiB")("copy", "Copy data rather than send via zero-copy");
    return app.run_deprecated(ac, av, [&app, &s] {
        auto &&config = app.configuration();
        auto chunk_size = config["chunk-size"].as<int>();
        auto mem_size = (size_t)config["mem-size"].as<int>() * MB;
        auto copy = config.count("copy");
        s.start(chunk_size, copy, mem_size);
    });
}
