//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2018-2020 Nil Foundation AG
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE net.basp.worker

#include <nil/mtl/network/basp/worker.hpp>

#include <nil/mtl/test/dsl.hpp>

#include <nil/mtl/actor_cast.hpp>
#include <nil/mtl/actor_control_block.hpp>
#include <nil/mtl/spawner.hpp>
#include <nil/mtl/serialization/binary_serializer.hpp>
#include <nil/mtl/make_actor.hpp>
#include <nil/mtl/network/basp/message_queue.hpp>
#include <nil/mtl/proxy_registry.hpp>

using namespace nil::mtl;

namespace {

    behavior testee_impl() {
        return {
            [](ok_atom) {
                // nop
            },
        };
    }

    class mock_actor_proxy : public actor_proxy {
    public:
        explicit mock_actor_proxy(actor_config &cfg) : actor_proxy(cfg) {
            // nop
        }

        void enqueue(mailbox_element_ptr, execution_unit *) override {
            BOOST_FAIL("mock_actor_proxy::enqueue called");
        }

        void kill_proxy(execution_unit *, error) override {
            // nop
        }
    };

    class mock_proxy_registry_backend : public proxy_registry::backend {
    public:
        mock_proxy_registry_backend(spawner &sys) : sys_(sys) {
            // nop
        }

        strong_actor_ptr make_proxy(node_id nid, actor_id aid) override {
            actor_config cfg;
            return make_actor<mock_actor_proxy, strong_actor_ptr>(aid, nid, &sys_, cfg);
        }

        void set_last_hop(node_id *) override {
            // nop
        }

    private:
        spawner &sys_;
    };

    struct fixture : test_coordinator_fixture<> {
        detail::worker_hub<net::basp::worker> hub;
        net::basp::message_queue queue;
        mock_proxy_registry_backend proxies_backend;
        proxy_registry proxies;
        node_id last_hop;
        actor testee;

        fixture() : proxies_backend(sys), proxies(sys, proxies_backend) {
            auto tmp = make_node_id(123, "0011223344556677889900112233445566778899");
            last_hop = unbox(std::move(tmp));
            testee = sys.spawn<lazy_init>(testee_impl);
            sys.registry().put(testee.id(), testee);
        }

        ~fixture() {
            sys.registry().erase(testee.id());
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(worker_tests, fixture)

BOOST_AUTO_TEST_CASE(deliver serialized message) {
    BOOST_TEST_MESSAGE("create the BASP worker");
    BOOST_REQUIRE_EQUAL(hub.peek(), nullptr);
    hub.add_new_worker(queue, proxies);
    MTL_REQUIRE_NOT_EQUAL(hub.peek(), nullptr);
    auto w = hub.pop();
    BOOST_TEST_MESSAGE("create a fake message + BASP header");
    std::vector<byte> payload;
    std::vector<strong_actor_ptr> stages;
    binary_serializer sink {sys, payload};
    if (auto err = sink(node_id {}, self->id(), testee.id(), stages, make_message(ok_atom_v)))
        BOOST_FAIL("unable to serialize message: " << sys.render(err));
    net::basp::header hdr {net::basp::message_type::actor_message, static_cast<uint32_t>(payload.size()),
                           make_message_id().integer_value()};
    BOOST_TEST_MESSAGE("launch worker");
    w->launch(last_hop, hdr, payload);
    sched.run_once();
    expect((ok_atom), from(_).to(testee));
}

BOOST_AUTO_TEST_SUITE_END()
