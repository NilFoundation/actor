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

#include <nil/actor/net/net.hh>
#include <nil/actor/core/reactor.hh>
#include <nil/actor/net/virtio.hh>
#include <iostream>

using namespace nil::actor;
using namespace net;

void dump_arp_packets(l3_protocol &proto) {
    // FIXME: ignored future
    (void)proto.receive(
        [](packet p, ethernet_address from) {
            std::cout << "seen arp packet\n";
            return make_ready_future<>();
        },
        [](forward_hash &out_hash_data, packet &p, size_t off) { return false; });
}

int main(int ac, char **av) {
    boost::program_options::variables_map opts;
    opts.insert(std::make_pair("tap-device", boost::program_options::variable_value(std::string("tap0"), false)));

    auto vnet = create_virtio_net_device(opts);
    interface netif(std::move(vnet));
    l3_protocol arp(&netif, eth_protocol_num::arp, [] { return std::optional<l3_protocol::l3packet>(); });
    dump_arp_packets(arp);
    engine().run();
    return 0;
}

