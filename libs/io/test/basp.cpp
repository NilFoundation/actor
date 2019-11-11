//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt for Boost License or
// http://opensource.org/licenses/BSD-3-Clause for BSD 3-Clause License
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE io_basp_tcp_test

#include <nil/mtl/config.hpp>
#include <nil/mtl/test/dsl.hpp>

#include <array>
#include <mutex>
#include <memory>
#include <limits>
#include <vector>
#include <iostream>
#include <condition_variable>

#include <nil/mtl/all.hpp>
#include <nil/mtl/io/all.hpp>

#include <nil/mtl/deep_to_string.hpp>

#include <nil/mtl/io/network/interfaces.hpp>
#include <nil/mtl/io/network/test_multiplexer.hpp>

namespace {

    using nil::mtl::make_message_id;

    struct anything {};

    anything any_vals;

    template<class T>
    struct maybe {
        maybe(T x) : val(std::move(x)) {
            // nop
        }

        maybe(anything) {
            // nop
        }

        nil::mtl::optional<T> val;
    };

    template<class T>
    std::string to_string(const maybe<T> &x) {
        return to_string(x.val);
    }

    template<class T>
    bool operator==(const maybe<T> &x, const T &y) {
        return x.val ? x.val == y : true;
    }

    constexpr uint8_t no_flags = 0;
    constexpr uint64_t no_operation_data = 0;
    constexpr uint64_t default_operation_data = make_message_id().integer_value();

    constexpr auto basp_atom = nil::mtl::atom("BASP");
    constexpr auto spawn_serv_atom = nil::mtl::atom("SpawnServ");
    constexpr auto config_serv_atom = nil::mtl::atom("ConfigServ");

}    // namespace

using namespace nil::mtl;
using namespace nil::mtl::io;

namespace {

    constexpr uint32_t num_remote_nodes = 2;

    using buffer = std::vector<char>;

    std::string hexstr(const buffer &buf) {
        return deep_to_string(meta::hex_formatted(), buf);
    }

    struct node {
        std::string name;
        node_id id;
        connection_handle connection;
        union {
            scoped_actor dummy_actor;
        };

        node() {
            // nop
        }

        ~node() {
            // nop
        }
    };

    class fixture {
        static inline actor_system_config &config(actor_system_config &cfg, bool autoconn = false) {
            cfg.middleman_enable_automatic_connections = autoconn;
            cfg.middleman_workers = 0;
            cfg.scheduler_policy = autoconn ? nil::mtl::atom("testing") : nil::mtl::atom("stealing");
            cfg.middleman_attach_utility_actors = autoconn;
            return cfg;
        }

    public:
        fixture(bool autoconn = false) : sys(config(cfg.load<io::middleman, network::test_multiplexer>(), autoconn)) {
            auto &mm = sys.middleman();
            mpx_ = dynamic_cast<network::test_multiplexer *>(&mm.backend());
            BOOST_REQUIRE(mpx_ != nullptr);
            BOOST_REQUIRE(&sys == &mpx_->system());
            auto hdl = mm.named_broker<basp_broker>(basp_atom);
            aut_ = static_cast<basp_broker *>(actor_cast<abstract_actor *>(hdl));
            this_node_ = sys.node();
            self_.reset(new scoped_actor {sys});
            ahdl_ = accept_handle::from_int(1);
            aut_->add_doorman(mpx_->new_doorman(ahdl_, 1u));
            registry_ = &sys.registry();
            registry_->put((*self_)->id(), actor_cast<strong_actor_ptr>(*self_));
            // first remote node is everything of this_node + 1, then +2, etc.
            auto pid = static_cast<node_id::default_data &>(*this_node_).process_id();
            auto hid = static_cast<node_id::default_data &>(*this_node_).host_id();
            for (uint32_t i = 0; i < num_remote_nodes; ++i) {
                auto &n = nodes_[i];
                for (auto &c : hid)
                    ++c;
                n.id = make_node_id(++pid, hid);
                n.connection = connection_handle::from_int(i + 1);
                new (&n.dummy_actor) scoped_actor(sys);
                // register all pseudo remote actors in the registry
                registry_->put(n.dummy_actor->id(), actor_cast<strong_actor_ptr>(n.dummy_actor));
            }
            // make sure all init messages are handled properly
            mpx_->flush_runnables();
            nodes_[0].name = "Jupiter";
            nodes_[1].name = "Mars";
            BOOST_REQUIRE(jupiter().connection != mars().connection);
            BOOST_TEST_MESSAGE("Earth:   " << to_string(this_node_));
            BOOST_TEST_MESSAGE("Jupiter: " << to_string(jupiter().id));
            BOOST_TEST_MESSAGE("Mars:    " << to_string(mars().id));
            BOOST_REQUIRE(this_node_ != jupiter().id);
            BOOST_REQUIRE(jupiter().id != mars().id);
        }

        ~fixture() {
            this_node_ = none;
            self_.reset();
            for (auto &n : nodes_) {
                n.id = none;
                n.dummy_actor.~scoped_actor();
            }
        }

        uint32_t serialized_size(const message &msg) {
            buffer buf;
            binary_serializer bs {mpx_, buf};
            auto e = bs(const_cast<message &>(msg));
            BOOST_REQUIRE(!e);
            return static_cast<uint32_t>(buf.size());
        }

        node &jupiter() {
            return nodes_[0];
        }

        node &mars() {
            return nodes_[1];
        }

        // our "virtual communication backend"
        network::test_multiplexer *mpx() {
            return mpx_;
        }

        // actor-under-test
        basp_broker *aut() {
            return aut_;
        }

        // our node ID
        node_id &this_node() {
            return this_node_;
        }

        // an actor reference representing a local actor
        scoped_actor &self() {
            return *self_;
        }

        // implementation of the Binary Actor System Protocol
        basp::instance &instance() {
            return aut()->instance;
        }

        // our routing table (filled by BASP)
        basp::routing_table &tbl() {
            return aut()->instance.tbl();
        }

        // access to proxy instances
        proxy_registry &proxies() {
            return aut()->proxies();
        }

        // stores the singleton pointer for convenience
        actor_registry *registry() {
            return registry_;
        }

        using payload_writer = basp::instance::payload_writer;

        template<class... Ts>
        void to_payload(binary_serializer &bs, const Ts &... xs) {
            bs(const_cast<Ts &>(xs)...);
        }

        template<class... Ts>
        void to_payload(buffer &buf, const Ts &... xs) {
            binary_serializer bs {mpx_, buf};
            to_payload(bs, xs...);
        }

        void to_buf(buffer &buf, basp::header &hdr, payload_writer *writer) {
            instance().write(mpx_, buf, hdr, writer);
        }

        template<class T, class... Ts>
        void to_buf(buffer &buf, basp::header &hdr, payload_writer *writer, const T &x, const Ts &... xs) {
            auto pw = make_callback([&](serializer &sink) -> error {
                if (writer)
                    return error::eval([&] { return (*writer)(sink); }, [&] { return sink(const_cast<T &>(x)); });
                return sink(const_cast<T &>(x));
            });
            to_buf(buf, hdr, &pw, xs...);
        }

        std::pair<basp::header, buffer> from_buf(const buffer &buf) {
            basp::header hdr;
            binary_deserializer bd {mpx_, buf};
            auto e = bd(hdr);
            BOOST_REQUIRE(!e);
            buffer payload;
            if (hdr.payload_len > 0) {
                std::copy(buf.begin() + basp::header_size, buf.end(), std::back_inserter(payload));
            }
            return {hdr, std::move(payload)};
        }

        void connect_node(node &n,
                          optional<accept_handle> ax = none,
                          actor_id published_actor_id = invalid_actor_id,
                          const std::set<std::string> &published_actor_ifs = std::set<std::string> {}) {
            auto src = ax ? *ax : ahdl_;
            BOOST_TEST_MESSAGE("connect remote node " << n.name << ", connection ID = " << n.connection.id()
                                                      << ", acceptor ID = " << src.id());
            auto hdl = n.connection;
            mpx_->add_pending_connect(src, hdl);
            mpx_->accept_connection(src);
            // technically, the server handshake arrives
            // before we send the client handshake
            mock(hdl, {basp::message_type::client_handshake, 0, 0, 0, invalid_actor_id, invalid_actor_id}, n.id)
                .receive(hdl, basp::message_type::server_handshake, no_flags, any_vals, basp::version, invalid_actor_id,
                         invalid_actor_id, this_node(), defaults::middleman::app_identifiers, published_actor_id,
                         published_actor_ifs)
                // upon receiving our client handshake, BASP will check
                // whether there is a SpawnServ actor on this node
                .receive(hdl, basp::message_type::direct_message, basp::header::named_receiver_flag, any_vals,
                         default_operation_data, any_vals, static_cast<uint64_t>(spawn_serv_atom),
                         std::vector<strong_actor_ptr> {}, make_message(sys_atom::value, get_atom::value, "info"));
            // test whether basp instance correctly updates the
            // routing table upon receiving client handshakes
            auto path = tbl().lookup(n.id);
            BOOST_REQUIRE(path);
            BOOST_CHECK(path->hdl == n.connection);
            BOOST_CHECK(path->next_hop == n.id);
        }

        std::pair<basp::header, buffer> read_from_out_buf(connection_handle hdl) {
            BOOST_TEST_MESSAGE("read from output buffer for connection " << hdl.id());
            auto &buf = mpx_->output_buffer(hdl);
            while (buf.size() < basp::header_size)
                mpx()->exec_runnable();
            auto result = from_buf(buf);
            buf.erase(buf.begin(), buf.begin() + basp::header_size + result.first.payload_len);
            return result;
        }

        void dispatch_out_buf(connection_handle hdl) {
            basp::header hdr;
            buffer buf;
            std::tie(hdr, buf) = read_from_out_buf(hdl);
            BOOST_TEST_MESSAGE("dispatch output buffer for connection " << hdl.id());
            BOOST_REQUIRE(hdr.operation == basp::message_type::direct_message);
            binary_deserializer source {mpx_, buf};
            std::vector<strong_actor_ptr> stages;
            message msg;
            auto e = source(stages, msg);
            BOOST_REQUIRE(!e);
            auto src = actor_cast<strong_actor_ptr>(registry_->get(hdr.source_actor));
            auto dest = registry_->get(hdr.dest_actor);
            BOOST_REQUIRE(dest);
            dest->enqueue(make_mailbox_element(src, make_message_id(), std::move(stages), std::move(msg)), nullptr);
        }

        class mock_t {
        public:
            mock_t(fixture *thisptr) : this_(thisptr) {
                // nop
            }

            mock_t(mock_t &&) = default;

            ~mock_t() {
                if (num > 1)
                    BOOST_TEST_MESSAGE("implementation under test responded with " << (num - 1) << " BASP message"
                                                                                   << (num > 2 ? "s" : ""));
            }

            template<class... Ts>
            mock_t &receive(connection_handle hdl,
                            maybe<basp::message_type>
                                operation,
                            maybe<uint8_t>
                                flags,
                            maybe<uint32_t>
                                payload_len,
                            maybe<uint64_t>
                                operation_data,
                            maybe<actor_id>
                                source_actor,
                            maybe<actor_id>
                                dest_actor,
                            const Ts &... xs) {
                BOOST_TEST_MESSAGE("expect #" << num);
                buffer buf;
                this_->to_payload(buf, xs...);
                buffer &ob = this_->mpx()->output_buffer(hdl);
                while (this_->mpx()->try_exec_runnable()) {
                    // repeat
                }
                BOOST_TEST_MESSAGE("output buffer has " << ob.size() << " bytes");
                basp::header hdr;
                {    // lifetime scope of source
                    binary_deserializer source {this_->mpx(), ob};
                    auto e = source(hdr);
                    BOOST_REQUIRE(e == none);
                }
                buffer payload;
                if (hdr.payload_len > 0) {
                    BOOST_REQUIRE(ob.size() >= (basp::header_size + hdr.payload_len));
                    auto first = ob.begin() + basp::header_size;
                    auto end = first + hdr.payload_len;
                    payload.assign(first, end);
                    BOOST_TEST_MESSAGE("erase " << std::distance(ob.begin(), end) << " bytes from output buffer");
                    ob.erase(ob.begin(), end);
                } else {
                    ob.erase(ob.begin(), ob.begin() + basp::header_size);
                }
                BOOST_CHECK(operation == hdr.operation);
                BOOST_CHECK(flags == static_cast<uint8_t>(hdr.flags));
                BOOST_CHECK(payload_len == hdr.payload_len);
                BOOST_CHECK(operation_data == hdr.operation_data);
                BOOST_CHECK(source_actor == hdr.source_actor);
                BOOST_CHECK(dest_actor == hdr.dest_actor);
                BOOST_REQUIRE_EQUAL(buf.size(), payload.size());
                BOOST_REQUIRE_EQUAL(hexstr(buf), hexstr(payload));
                ++num;
                return *this;
            }

        private:
            fixture *this_;
            size_t num = 1;
        };

        template<class... Ts>
        mock_t mock(connection_handle hdl, basp::header hdr, const Ts &... xs) {
            buffer buf;
            to_buf(buf, hdr, nullptr, xs...);
            BOOST_TEST_MESSAGE("virtually send " << to_string(hdr.operation) << " with "
                                                 << (buf.size() - basp::header_size) << " bytes payload");
            mpx()->virtual_send(hdl, buf);
            return {this};
        }

        mock_t mock() {
            return {this};
        }

        actor_system_config cfg;
        actor_system sys;

    private:
        basp_broker *aut_;
        accept_handle ahdl_;
        network::test_multiplexer *mpx_;
        node_id this_node_;
        std::unique_ptr<scoped_actor> self_;
        std::array<node, num_remote_nodes> nodes_;
        /*
        array<node_id, num_remote_nodes> remote_node_;
        array<connection_handle, num_remote_nodes> remote_hdl_;
        array<unique_ptr<scoped_actor>, num_remote_nodes> pseudo_remote_;
        */
        actor_registry *registry_;
    };

    class autoconn_enabled_fixture : public fixture {
    public:
        using scheduler_type = nil::mtl::scheduler::test_coordinator;

        scheduler_type &sched;
        middleman_actor mma;

        autoconn_enabled_fixture() :
            fixture(true), sched(dynamic_cast<scheduler_type &>(sys.scheduler())), mma(sys.middleman().actor_handle()) {
            // nop
        }

        void publish(const actor &whom, uint16_t port) {
            using sig_t = std::set<std::string>;
            scoped_actor tmp {sys};
            sig_t sigs;
            tmp->send(mma, publish_atom::value, port, actor_cast<strong_actor_ptr>(whom), std::move(sigs), "", false);
            expect((atom_value, uint16_t, strong_actor_ptr, sig_t, std::string, bool), from(tmp).to(mma));
            expect((uint16_t), from(mma).to(tmp).with(port));
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(basp_tests, fixture)

BOOST_AUTO_TEST_CASE(empty_server_handshake) {
    // test whether basp instance correctly sends a
    // server handshake when there's no actor published
    buffer buf;
    instance().write_server_handshake(mpx(), buf, none);
    basp::header hdr;
    buffer payload;
    std::tie(hdr, payload) = from_buf(buf);
    basp::header expected {basp::message_type::server_handshake,
                           0,
                           static_cast<uint32_t>(payload.size()),
                           basp::version,
                           invalid_actor_id,
                           invalid_actor_id};
    BOOST_CHECK(basp::valid(hdr));
    BOOST_CHECK(basp::is_handshake(hdr));
    BOOST_CHECK_EQUAL(to_string(hdr), to_string(expected));
}

BOOST_AUTO_TEST_CASE(non_empty_server_handshake) {
    // test whether basp instance correctly sends a
    // server handshake with published actors
    buffer buf;
    instance().add_published_actor(4242, actor_cast<strong_actor_ptr>(self()),
                                   {"nil::mtl::replies_to<@u16>::with<@u16>"});
    instance().write_server_handshake(mpx(), buf, uint16_t {4242});
    basp::header hdr;
    buffer payload;
    std::tie(hdr, payload) = from_buf(buf);
    basp::header expected {basp::message_type::server_handshake,
                           0,
                           static_cast<uint32_t>(payload.size()),
                           basp::version,
                           invalid_actor_id,
                           invalid_actor_id};
    BOOST_CHECK(basp::valid(hdr));
    BOOST_CHECK(basp::is_handshake(hdr));
    BOOST_CHECK_EQUAL(to_string(hdr), to_string(expected));
    buffer expected_payload;
    binary_serializer bd {nullptr, expected_payload};
    bd(instance().this_node(), defaults::middleman::app_identifiers, self()->id(),
       std::set<std::string> {"nil::mtl::replies_to<@u16>::with<@u16>"});
    BOOST_CHECK_EQUAL(hexstr(payload), hexstr(expected_payload));
}

BOOST_AUTO_TEST_CASE(remote_address_and_port) {
    BOOST_TEST_MESSAGE("connect to Mars");
    connect_node(mars());
    auto mm = sys.middleman().actor_handle();
    BOOST_TEST_MESSAGE("ask MM about node ID of Mars");
    self()->send(mm, get_atom::value, mars().id);
    do {
        mpx()->exec_runnable();
    } while (self()->mailbox().empty());
    BOOST_TEST_MESSAGE("receive result of MM");
    self()->receive([&](const node_id &nid, const std::string &addr, uint16_t port) {
        BOOST_CHECK(nid == mars().id);
        // all test nodes have address "test" and connection handle ID as port
        BOOST_CHECK_EQUAL(addr, "test");
        BOOST_CHECK_EQUAL(port, mars().connection.id());
    });
}

BOOST_AUTO_TEST_CASE(client_handshake_and_dispatch) {
    BOOST_TEST_MESSAGE("connect to Jupiter");
    connect_node(jupiter());
    // send a message via `dispatch` from node 0
    mock(jupiter().connection, {basp::message_type::direct_message, 0, 0, 0, jupiter().dummy_actor->id(), self()->id()},
         std::vector<actor_addr> {}, make_message(1, 2, 3))
        .receive(jupiter().connection, basp::message_type::monitor_message, no_flags, any_vals, no_operation_data,
                 invalid_actor_id, jupiter().dummy_actor->id(), this_node(), jupiter().id);
    // must've created a proxy for our remote actor
    BOOST_REQUIRE(proxies().count_proxies(jupiter().id) == 1);
    // must've send remote node a message that this proxy is monitored now
    // receive the message
    self()->receive([](int a, int b, int c) {
        BOOST_CHECK_EQUAL(a, 1);
        BOOST_CHECK_EQUAL(b, 2);
        BOOST_CHECK_EQUAL(c, 3);
        return a + b + c;
    });
    BOOST_TEST_MESSAGE("exec message of forwarding proxy");
    mpx()->exec_runnable();
    // deserialize and send message from out buf
    dispatch_out_buf(jupiter().connection);
    jupiter().dummy_actor->receive([](int i) { BOOST_CHECK_EQUAL(i, 6); });
}

BOOST_AUTO_TEST_CASE(message_forwarding) {
    // connect two remote nodes
    connect_node(jupiter());
    connect_node(mars());
    auto msg = make_message(1, 2, 3);
    // send a message from node 0 to node 1, forwarded by this node
    mock(jupiter().connection,
         {basp::message_type::routed_message, 0, 0, default_operation_data, invalid_actor_id, mars().dummy_actor->id()},
         jupiter().id, mars().id, std::vector<strong_actor_ptr> {}, msg)
        .receive(mars().connection, basp::message_type::routed_message, no_flags, any_vals, default_operation_data,
                 invalid_actor_id, mars().dummy_actor->id(), jupiter().id, mars().id, std::vector<strong_actor_ptr> {},
                 msg);
}

BOOST_AUTO_TEST_CASE(publish_and_connect) {
    auto ax = accept_handle::from_int(4242);
    mpx()->provide_acceptor(4242, ax);
    auto res = sys.middleman().publish(self(), 4242);
    BOOST_REQUIRE(res == 4242);
    mpx()->flush_runnables();    // process publish message in basp_broker
    connect_node(jupiter(), ax, self()->id());
}

BOOST_AUTO_TEST_CASE(remote_actor_and_send) {
    constexpr const char *lo = "localhost";
    BOOST_TEST_MESSAGE("self: " << to_string(self()->address()));
    mpx()->provide_scribe(lo, 4242, jupiter().connection);
    BOOST_REQUIRE(mpx()->has_pending_scribe(lo, 4242));
    auto mm1 = sys.middleman().actor_handle();
    actor result;
    auto f = self()->request(mm1, infinite, connect_atom::value, lo, uint16_t {4242});
    // wait until BASP broker has received and processed the connect message
    while (!aut()->valid(jupiter().connection))
        mpx()->exec_runnable();
    BOOST_REQUIRE(!mpx()->has_pending_scribe(lo, 4242));
    // build a fake server handshake containing the id of our first pseudo actor
    BOOST_TEST_MESSAGE("server handshake => client handshake + proxy announcement");
    auto na = registry()->named_actors();
    mock(jupiter().connection,
         {basp::message_type::server_handshake, 0, 0, basp::version, invalid_actor_id, invalid_actor_id}, jupiter().id,
         defaults::middleman::app_identifiers, jupiter().dummy_actor->id(), std::set<std::string> {})
        .receive(jupiter().connection, basp::message_type::client_handshake, no_flags, any_vals, no_operation_data,
                 invalid_actor_id, invalid_actor_id, this_node())
        .receive(jupiter().connection, basp::message_type::direct_message, basp::header::named_receiver_flag, any_vals,
                 default_operation_data, any_vals, static_cast<uint64_t>(spawn_serv_atom),
                 std::vector<strong_actor_ptr> {}, make_message(sys_atom::value, get_atom::value, "info"))
        .receive(jupiter().connection, basp::message_type::monitor_message, no_flags, any_vals, no_operation_data,
                 invalid_actor_id, jupiter().dummy_actor->id(), this_node(), jupiter().id);
    BOOST_TEST_MESSAGE("BASP broker should've send the proxy");
    f.receive(
        [&](node_id nid, strong_actor_ptr res, std::set<std::string> ifs) {
            BOOST_REQUIRE(res);
            auto aptr = actor_cast<abstract_actor *>(res);
            BOOST_REQUIRE(dynamic_cast<forwarding_actor_proxy *>(aptr) != nullptr);
            BOOST_CHECK_EQUAL(proxies().count_proxies(jupiter().id), 1u);
            BOOST_CHECK(nid == jupiter().id);
            BOOST_CHECK(res->node() == jupiter().id);
            BOOST_CHECK_EQUAL(res->id(), jupiter().dummy_actor->id());
            BOOST_CHECK(ifs.empty());
            auto proxy = proxies().get(jupiter().id, jupiter().dummy_actor->id());
            BOOST_REQUIRE(proxy != nullptr);
            BOOST_REQUIRE(proxy == res);
            result = actor_cast<actor>(res);
        },
        [&](error &err) { BOOST_FAIL("error: " << sys.render(err)); });
    BOOST_TEST_MESSAGE("send message to proxy");
    anon_send(actor_cast<actor>(result), 42);
    mpx()->flush_runnables();
    //  mpx()->exec_runnable(); // process forwarded message in basp_broker
    mock().receive(jupiter().connection, basp::message_type::direct_message, no_flags, any_vals, default_operation_data,
                   invalid_actor_id, jupiter().dummy_actor->id(), std::vector<strong_actor_ptr> {}, make_message(42));
    auto msg = make_message("hi there!");
    BOOST_TEST_MESSAGE("send message via BASP (from proxy)");
    mock(jupiter().connection, {basp::message_type::direct_message, 0, 0, 0, jupiter().dummy_actor->id(), self()->id()},
         std::vector<strong_actor_ptr> {}, make_message("hi there!"));
    self()->receive([&](const std::string &str) {
        BOOST_CHECK_EQUAL(to_string(self()->current_sender()), to_string(result));
        BOOST_CHECK(self()->current_sender() == result.address());
        BOOST_CHECK_EQUAL(str, "hi there!");
    });
}

BOOST_AUTO_TEST_CASE(actor_serialize_and_deserialize) {
    auto testee_impl = [](event_based_actor *testee_self) -> behavior {
        testee_self->set_default_handler(reflect_and_quit);
        return {[] {
            // nop
        }};
    };
    connect_node(jupiter());
    auto prx = proxies().get_or_put(jupiter().id, jupiter().dummy_actor->id());
    mock().receive(jupiter().connection, basp::message_type::monitor_message, no_flags, any_vals, no_operation_data,
                   invalid_actor_id, prx->id(), this_node(), prx->node());
    BOOST_CHECK(prx->node() == jupiter().id);
    BOOST_CHECK(prx->id() == jupiter().dummy_actor->id());
    auto testee = sys.spawn(testee_impl);
    registry()->put(testee->id(), actor_cast<strong_actor_ptr>(testee));
    BOOST_TEST_MESSAGE("send message via BASP (from proxy)");
    auto msg = make_message(actor_cast<actor_addr>(prx));
    mock(jupiter().connection, {basp::message_type::direct_message, 0, 0, 0, prx->id(), testee->id()},
         std::vector<strong_actor_ptr> {}, msg);
    // testee must've responded (process forwarded message in BASP broker)
    BOOST_TEST_MESSAGE("wait until BASP broker writes to its output buffer");
    while (mpx()->output_buffer(jupiter().connection).empty())
        mpx()->exec_runnable();    // process forwarded message in basp_broker
    // output buffer must contain the reflected message
    mock().receive(jupiter().connection, basp::message_type::direct_message, no_flags, any_vals, default_operation_data,
                   testee->id(), prx->id(), std::vector<strong_actor_ptr> {}, msg);
}

BOOST_AUTO_TEST_CASE(indirect_connections) {
    // this node receives a message from jupiter via mars and responds via mars
    // and any ad-hoc automatic connection requests are ignored
    BOOST_TEST_MESSAGE("self: " << to_string(self()->address()));
    BOOST_TEST_MESSAGE("publish self at port 4242");
    auto ax = accept_handle::from_int(4242);
    mpx()->provide_acceptor(4242, ax);
    sys.middleman().publish(self(), 4242);
    mpx()->flush_runnables();    // process publish message in basp_broker
    BOOST_TEST_MESSAGE("connect to Mars");
    connect_node(mars(), ax, self()->id());
    BOOST_TEST_MESSAGE("actor from Jupiter sends a message to us via Mars");
    auto mx = mock(mars().connection,
                   {basp::message_type::routed_message, 0, 0, 0, jupiter().dummy_actor->id(), self()->id()},
                   jupiter().id, this_node(), std::vector<actor_id> {}, make_message("hello from jupiter!"));
    BOOST_TEST_MESSAGE("expect ('sys', 'get', \"info\") from Earth to Jupiter at Mars");
    // this asks Jupiter if it has a 'SpawnServ'
    mx.receive(mars().connection, basp::message_type::routed_message, basp::header::named_receiver_flag, any_vals,
               default_operation_data, any_vals, static_cast<uint64_t>(spawn_serv_atom), this_node(), jupiter().id,
               std::vector<strong_actor_ptr> {}, make_message(sys_atom::value, get_atom::value, "info"));
    BOOST_TEST_MESSAGE("expect announce_proxy message at Mars from Earth to Jupiter");
    mx.receive(mars().connection, basp::message_type::monitor_message, no_flags, any_vals, no_operation_data,
               invalid_actor_id, jupiter().dummy_actor->id(), this_node(), jupiter().id);
    BOOST_TEST_MESSAGE("receive message from jupiter");
    self()->receive([](const std::string &str) -> std::string {
        BOOST_CHECK_EQUAL(str, "hello from jupiter!");
        return "hello from earth!";
    });
    mpx()->exec_runnable();    // process forwarded message in basp_broker
    mock().receive(mars().connection, basp::message_type::routed_message, no_flags, any_vals, default_operation_data,
                   self()->id(), jupiter().dummy_actor->id(), this_node(), jupiter().id,
                   std::vector<strong_actor_ptr> {}, make_message("hello from earth!"));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(basp_tests_with_autoconn, autoconn_enabled_fixture)

BOOST_AUTO_TEST_CASE(automatic_connection) {
    // Utility helper for verifying routing tables.
    auto check_node_in_tbl = [&](node &n) {
        auto hdl = tbl().lookup_direct(n.id);
        BOOST_REQUIRE(hdl);
    };
    // Setup.
    mpx()->provide_scribe("jupiter", 8080, jupiter().connection);
    BOOST_CHECK(mpx()->has_pending_scribe("jupiter", 8080));
    BOOST_TEST_MESSAGE("self: " << to_string(self()->address()));
    auto ax = accept_handle::from_int(4242);
    mpx()->provide_acceptor(4242, ax);
    publish(self(), 4242);
    // Process publish message in basp_broker.
    mpx()->flush_runnables();
    BOOST_TEST_MESSAGE("connect to mars");
    connect_node(mars(), ax, self()->id());
    check_node_in_tbl(mars());
    BOOST_TEST_MESSAGE("simulate that a message from jupiter travels over mars");
    mock(mars().connection,
         {basp::message_type::routed_message, 0, 0, make_message_id().integer_value(), jupiter().dummy_actor->id(),
          self()->id()},
         jupiter().id, this_node(), std::vector<strong_actor_ptr> {}, make_message("hello from jupiter!"))
        .receive(mars().connection, basp::message_type::routed_message, basp::header::named_receiver_flag, any_vals,
                 default_operation_data, any_vals, static_cast<uint64_t>(spawn_serv_atom), this_node(), jupiter().id,
                 std::vector<strong_actor_ptr> {}, make_message(sys_atom::value, get_atom::value, "info"))
        .receive(mars().connection, basp::message_type::routed_message, basp::header::named_receiver_flag, any_vals,
                 default_operation_data,
                 any_vals,    // actor ID of an actor spawned by the BASP broker
                 static_cast<uint64_t>(config_serv_atom), this_node(), jupiter().id, std::vector<strong_actor_ptr> {},
                 make_message(get_atom::value, "basp.default-connectivity-tcp"))
        .receive(mars().connection, basp::message_type::monitor_message, no_flags, any_vals, no_operation_data,
                 invalid_actor_id, jupiter().dummy_actor->id(), this_node(), jupiter().id);
    BOOST_CHECK_EQUAL(mpx()->output_buffer(mars().connection).size(), 0u);
    BOOST_CHECK(tbl().lookup_indirect(jupiter().id) == mars().id);
    BOOST_CHECK(tbl().lookup_indirect(mars().id) == none);
    auto connection_helper_actor = sys.latest_actor_id();
    BOOST_CHECK_EQUAL(mpx()->output_buffer(mars().connection).size(), 0u);
    // Create a dummy config server and respond to the name lookup.
    BOOST_TEST_MESSAGE("receive ConfigServ of jupiter");
    network::address_listing res;
    res[network::protocol::ipv4].emplace_back("jupiter");
    mock(mars().connection,
         {basp::message_type::routed_message, 0, 0, make_message_id().integer_value(), invalid_actor_id,
          connection_helper_actor},
         this_node(), this_node(), std::vector<strong_actor_ptr> {},
         make_message("basp.default-connectivity-tcp", make_message(uint16_t {8080}, std::move(res))));
    // Our connection helper should now connect to jupiter and send the scribe
    // handle over to the BASP broker.
    while (mpx()->has_pending_scribe("jupiter", 8080)) {
        sched.run();
        mpx()->flush_runnables();
    }
    BOOST_REQUIRE(mpx()->output_buffer(mars().connection).empty());
    // Send handshake from jupiter.
    mock(jupiter().connection,
         {basp::message_type::server_handshake, no_flags, 0, basp::version, invalid_actor_id, invalid_actor_id},
         jupiter().id, defaults::middleman::app_identifiers, jupiter().dummy_actor->id(), std::set<std::string> {})
        .receive(jupiter().connection, basp::message_type::client_handshake, no_flags, any_vals, no_operation_data,
                 invalid_actor_id, invalid_actor_id, this_node());
    BOOST_CHECK(tbl().lookup_indirect(jupiter().id) == none);
    BOOST_CHECK(tbl().lookup_indirect(mars().id) == none);
    check_node_in_tbl(jupiter());
    check_node_in_tbl(mars());
    BOOST_TEST_MESSAGE("receive message from jupiter");
    self()->receive([](const std::string &str) -> std::string {
        BOOST_CHECK_EQUAL(str, "hello from jupiter!");
        return "hello from earth!";
    });
    mpx()->exec_runnable();    // process forwarded message in basp_broker
    BOOST_TEST_MESSAGE("response message must take direct route now");
    mock().receive(jupiter().connection, basp::message_type::direct_message, no_flags, any_vals,
                   make_message_id().integer_value(), self()->id(), jupiter().dummy_actor->id(),
                   std::vector<strong_actor_ptr> {}, make_message("hello from earth!"));
    BOOST_CHECK_EQUAL(mpx()->output_buffer(mars().connection).size(), 0u);
}

BOOST_AUTO_TEST_SUITE_END()
