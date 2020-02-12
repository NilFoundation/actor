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

#define BOOST_TEST_MODULE stream_transport

#include <nil/actor/network/stream_transport.hpp>

#include <nil/actor/test/host_fixture.hpp>
#include <nil/actor/test/dsl.hpp>

#include <nil/actor/serialization/binary_deserializer.hpp>
#include <nil/actor/serialization/binary_serializer.hpp>
#include <nil/actor/byte.hpp>
#include <nil/actor/detail/scope_guard.hpp>
#include <nil/actor/make_actor.hpp>
#include <nil/actor/network/actor_proxy_impl.hpp>
#include <nil/actor/network/endpoint_manager.hpp>
#include <nil/actor/network/endpoint_manager_impl.hpp>
#include <nil/actor/network/make_endpoint_manager.hpp>
#include <nil/actor/network/multiplexer.hpp>
#include <nil/actor/network/socket_guard.hpp>
#include <nil/actor/network/stream_socket.hpp>
#include <nil/actor/span.hpp>

using namespace nil::actor;
using namespace nil::actor::network;

namespace {
    constexpr string_view hello_manager = "hello manager!";

    struct fixture : test_coordinator_fixture<>, host_fixture {
        using buffer_type = std::vector<byte>;

        using buffer_ptr = std::shared_ptr<buffer_type>;

        fixture() : recv_buf(1024), shared_buf {std::make_shared<buffer_type>()} {
            mpx = std::make_shared<multiplexer>();
            if (auto err = mpx->init())
                BOOST_FAIL("mpx->init failed: " << sys.render(err));
            mpx->set_thread_id();
            BOOST_CHECK_EQUAL(mpx->num_socket_managers(), 1u);
            auto sockets = unbox(make_stream_socket_pair());
            send_socket_guard.reset(sockets.first);
            recv_socket_guard.reset(sockets.second);
            if (auto err = nonblocking(recv_socket_guard.socket(), true))
                BOOST_FAIL("nonblocking returned an error: " << err);
        }

        bool handle_io_event() override {
            return mpx->poll_once(false);
        }

        multiplexer_ptr mpx;
        buffer_type recv_buf;
        socket_guard<stream_socket> send_socket_guard;
        socket_guard<stream_socket> recv_socket_guard;
        buffer_ptr shared_buf;
    };

    class dummy_application {
        using buffer_type = std::vector<byte>;

        using buffer_ptr = std::shared_ptr<buffer_type>;

    public:
        dummy_application(buffer_ptr rec_buf) :
            rec_buf_(std::move(rec_buf)) {
                // nop
            };

        ~dummy_application() = default;

        template<class Parent>
        error init(Parent &) {
            return none;
        }

        template<class Parent>
        void write_message(Parent &parent, std::unique_ptr<endpoint_manager_queue::message> ptr) {
            parent.write_packet(ptr->payload);
        }

        template<class Parent>
        error handle_data(Parent &, span<const byte> data) {
            rec_buf_->clear();
            rec_buf_->insert(rec_buf_->begin(), data.begin(), data.end());
            return none;
        }

        template<class Parent>
        void resolve(Parent &parent, string_view path, const actor &listener) {
            actor_id aid = 42;
            auto hid = "0011223344556677889900112233445566778899";
            auto nid = unbox(make_node_id(42, hid));
            actor_config cfg;
            endpoint_manager_ptr ptr {&parent.manager()};
            auto p = make_actor<actor_proxy_impl, strong_actor_ptr>(aid, nid, &parent.system(), cfg, std::move(ptr));
            anon_send(listener, resolve_atom_v, std::string {path.begin(), path.end()}, p);
        }

        template<class Parent>
        void timeout(Parent &, const std::string &, uint64_t) {
            // nop
        }

        template<class Parent>
        void new_proxy(Parent &, actor_id) {
            // nop
        }

        template<class Parent>
        void local_actor_down(Parent &, actor_id, error) {
            // nop
        }

        void handle_error(sec) {
            // nop
        }

        static expected<buffer_type> serialize(spawner &sys, const type_erased_tuple &x) {
            buffer_type result;
            binary_serializer sink {sys, result};
            if (auto err = message::save(sink, x))
                return err.value();
            return result;
        }

    private:
        buffer_ptr rec_buf_;
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(endpoint_manager_tests, fixture)

BOOST_AUTO_TEST_CASE(receive) {
    using transport_type = stream_transport<dummy_application>;
    auto mgr =
        make_endpoint_manager(mpx, sys, transport_type {recv_socket_guard.release(), dummy_application {shared_buf}});
    BOOST_CHECK_EQUAL(mgr->init(), none);
    auto mgr_impl = mgr.downcast<endpoint_manager_impl<transport_type>>();
    BOOST_CHECK(mgr_impl != nullptr);
    auto &transport = mgr_impl->transport();
    transport.configure_read(receive_policy::exactly(hello_manager.size()));
    BOOST_CHECK_EQUAL(mpx->num_socket_managers(), 2u);
    BOOST_CHECK_EQUAL(write(send_socket_guard.socket(), as_bytes(make_span(hello_manager))), hello_manager.size());
    BOOST_TEST_MESSAGE("wrote " << hello_manager.size() << " bytes.");
    run();
    BOOST_CHECK_EQUAL(string_view(reinterpret_cast<char *>(shared_buf->data()), shared_buf->size()), hello_manager);
}

BOOST_AUTO_TEST_CASE(resolve and proxy communication) {
    using transport_type = stream_transport<dummy_application>;
    auto mgr =
        make_endpoint_manager(mpx, sys, transport_type {send_socket_guard.release(), dummy_application {shared_buf}});
    BOOST_CHECK_EQUAL(mgr->init(), none);
    run();
    mgr->resolve(unbox(make_uri("test:/id/42")), self);
    run();
    self->receive(
        [&](resolve_atom, const std::string &, const strong_actor_ptr &p) {
            BOOST_TEST_MESSAGE("got a proxy, send a message to it");
            self->send(actor_cast<actor>(p), "hello proxy!");
        },
        after(std::chrono::seconds(0)) >> [&] { BOOST_FAIL("manager did not respond with a proxy."); });
    run();
    auto read_res = read(recv_socket_guard.socket(), recv_buf);
    if (!holds_alternative<size_t>(read_res))
        BOOST_FAIL("read() returned an error: " << sys.render(get<sec>(read_res)));
    recv_buf.resize(get<size_t>(read_res));
    BOOST_TEST_MESSAGE("receive buffer contains " << recv_buf.size() << " bytes");
    message msg;
    binary_deserializer source {sys, recv_buf};
    BOOST_CHECK_EQUAL(source(msg), none);
    if (msg.match_elements<std::string>())
        BOOST_CHECK_EQUAL(msg.get_as<std::string>(0), "hello proxy!");
    else
        ACTOR_ERROR("expected a string, got: " << to_string(msg));
}

BOOST_AUTO_TEST_SUITE_END()
