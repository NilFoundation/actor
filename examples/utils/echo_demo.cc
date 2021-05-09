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

#include <nil/actor/network/virtio.hh>
#include <nil/actor/network/dpdk.hh>
#include <nil/actor/core/reactor.hh>
#include <nil/actor/network/ip.hh>
#include <iostream>
#include <utility>
#include <algorithm>

using namespace nil::actor;
using namespace net;

void dump_packet(const packet &p) {
    std::cout << "rx:";
    auto f = p.frag(0);
    for (unsigned i = 0; i < std::min(f.size, size_t(30)); ++i) {
        char x[4];
        std::sprintf(x, " %02x", uint8_t(f.base[i]));
        std::cout << x;
    }
    std::cout << "\n";
}

future<> echo_packet(net::qp &netif, packet p) {
    auto f = p.frag(0);
    if (f.size < sizeof(eth_hdr)) {
        return make_ready_future<>();
    }
    auto pos = 0;
    auto eh = reinterpret_cast<eth_hdr *>(f.base + pos);
    pos += sizeof(*eh);
    *eh = ntoh(*eh);
    if (eh->eth_proto != 0x0800) {
        return make_ready_future<>();
    }
    auto iph = reinterpret_cast<ip_hdr *>(f.base + pos);
    *iph = ntoh(*iph);
    pos += iph->ihl * 4;
    if (iph->ver != 4 || iph->ihl < 5 || iph->ip_proto != 1) {
        return make_ready_future<>();
    }
    auto ip_len = iph->len;
    auto icmph = reinterpret_cast<icmp_hdr *>(f.base + pos);
    if (icmph->type != icmp_hdr::msg_type::echo_request) {
        return make_ready_future<>();
    }
    auto icmp_len = ip_len - iph->ihl * 4;
    std::swap(eh->src_mac, eh->dst_mac);
    std::swap(iph->src_ip, iph->dst_ip);
    icmph->type = icmp_hdr::msg_type::echo_reply;
    icmph->csum = 0;
    *iph = hton(*iph);
    *eh = hton(*eh);
    icmph->csum = ip_checksum(icmph, icmp_len);
    iph->csum = 0;
    iph->csum = ip_checksum(iph, iph->ihl * 4);
    return netif.send(std::move(p));
}

#ifdef ACTOR_HAVE_DPDK
void usage() {
    std::cout << "Usage: echotest [-virtio|-dpdk]" << std::endl;
    std::cout << "   -virtio - use virtio backend (default)" << std::endl;
    std::cout << "   -dpdk   - use dpdk-pmd backend" << std::endl;
}
#endif

int main(int ac, char **av) {
    std::unique_ptr<net::device> dnet;
    net::qp *vnet;

    boost::program_options::variables_map opts;
    opts.insert(std::make_pair("tap-device", boost::program_options::variable_value(std::string("tap0"), false)));

#ifdef ACTOR_HAVE_DPDK
    if (ac > 2) {
        usage();
        return -1;
    }

    if ((ac == 1) || !std::strcmp(av[1], "-virtio")) {
        dnet = create_virtio_net_device(opts);
    } else if (!std::strcmp(av[1], "-dpdk")) {
        dnet = create_dpdk_net_device();
    } else {
        usage();
        return -1;
    }
#else
    dnet = create_virtio_net_device(opts);
#endif    // ACTOR_HAVE_DPDK

    auto qp = dnet->init_local_queue(opts, 0);
    vnet = qp.get();
    dnet->set_local_queue(std::move(qp));
    future<> rx_done = dnet->receive([vnet](packet p) { return echo_packet(*vnet, std::move(p)); });
    engine().run();
    return 0;
}

