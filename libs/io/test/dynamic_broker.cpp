#include <nil/actor/config.hpp>

#define BOOST_TEST_MODULE io_dynamic_broker_test

#include <nil/actor/test/dsl.hpp>

#include <memory>
#include <iostream>

#include <nil/actor/all.hpp>
#include <nil/actor/io/all.hpp>

#include <nil/actor/string_algorithms.hpp>

using namespace std;
using namespace nil::actor;
using namespace nil::actor::io;

namespace {

    using ping_atom = nil::actor::atom_constant<nil::actor::atom("ping")>;
    using pong_atom = nil::actor::atom_constant<nil::actor::atom("pong")>;
    using publish_atom = nil::actor::atom_constant<nil::actor::atom("publish")>;
    using kickoff_atom = nil::actor::atom_constant<nil::actor::atom("kickoff")>;

    void ping(event_based_actor *self, size_t num_pings) {
        BOOST_TEST_MESSAGE("num_pings: " << num_pings);
        auto count = std::make_shared<size_t>(0);
        self->become([=](kickoff_atom, const actor &pong) {
            BOOST_TEST_MESSAGE("received `kickoff_atom`");
            self->send(pong, ping_atom::value, 1);
            self->become([=](pong_atom, int value) -> std::tuple<atom_value, int> {
                if (++*count >= num_pings) {
                    BOOST_TEST_MESSAGE("received " << num_pings << " pings, call self->quit");
                    self->quit();
                }
                return std::make_tuple(ping_atom::value, value + 1);
            });
        });
    }

    void pong(event_based_actor *self) {
        BOOST_TEST_MESSAGE("pong actor started");
        self->set_down_handler([=](down_msg &dm) {
            BOOST_TEST_MESSAGE("received down_msg{" << to_string(dm.reason) << "}");
            self->quit(dm.reason);
        });
        self->become([=](ping_atom, int value) -> std::tuple<atom_value, int> {
            BOOST_TEST_MESSAGE("received `ping_atom`");
            self->monitor(self->current_sender());
            // set next behavior
            self->become([](ping_atom, int val) { return std::make_tuple(pong_atom::value, val); });
            // reply to 'ping'
            return std::make_tuple(pong_atom::value, value);
        });
    }

    void peer_fun(broker *self, connection_handle hdl, const actor &buddy) {
        BOOST_TEST_MESSAGE("peer_fun called");
        BOOST_REQUIRE(self->subtype() == resumable::io_actor);
        BOOST_CHECK(self != nullptr);
        self->monitor(buddy);
        // assume exactly one connection
        BOOST_REQUIRE(self->connections().size() == 1);
        self->configure_read(hdl, receive_policy::exactly(sizeof(atom_value) + sizeof(int)));
        auto write = [=](atom_value type, int value) {
            auto &buf = self->wr_buf(hdl);
            auto first = reinterpret_cast<char *>(&type);
            buf.insert(buf.end(), first, first + sizeof(atom_value));
            first = reinterpret_cast<char *>(&value);
            buf.insert(buf.end(), first, first + sizeof(int));
            self->flush(hdl);
        };
        self->set_down_handler([=](down_msg &dm) {
            BOOST_TEST_MESSAGE("received: " << to_string(dm));
            if (dm.source == buddy) {
                self->quit(dm.reason);
            }
        });
        self->become(
            [=](const connection_closed_msg &) {
                BOOST_TEST_MESSAGE("received connection_closed_msg");
                self->quit();
            },
            [=](const new_data_msg &msg) {
                BOOST_TEST_MESSAGE("received new_data_msg");
                atom_value type;
                int value;
                memcpy(&type, msg.buf.data(), sizeof(atom_value));
                memcpy(&value, msg.buf.data() + sizeof(atom_value), sizeof(int));
                self->send(buddy, type, value);
            },
            [=](ping_atom, int value) {
                BOOST_TEST_MESSAGE("received: ping " << value);
                write(ping_atom::value, value);
            },
            [=](pong_atom, int value) {
                BOOST_TEST_MESSAGE("received: pong " << value);
                write(pong_atom::value, value);
            });
    }

    behavior peer_acceptor_fun(broker *self, const actor &buddy) {
        BOOST_TEST_MESSAGE("peer_acceptor_fun");
        return {[=](const new_connection_msg &msg) {
                    BOOST_TEST_MESSAGE("received `new_connection_msg`");
                    self->fork(peer_fun, msg.handle, buddy);
                    self->quit();
                },
                [=](publish_atom) -> expected<uint16_t> {
                    auto res = self->add_tcp_doorman(0, "127.0.0.1");
                    if (!res) {
                        return std::move(res.error());
                    }
                    return res->second;
                }};
    }

    void run_client(uint16_t port) {
        spawner_config cfg;
        spawner system {cfg.load<io::middleman>()};
        auto p = system.spawn(ping, size_t {10});
        BOOST_TEST_MESSAGE("spawn_client...");
        auto cl = unbox(system.middleman().spawn_client(peer_fun, "127.0.0.1", port, p));
        BOOST_TEST_MESSAGE("spawn_client finished");
        anon_send(p, kickoff_atom::value, cl);
        BOOST_TEST_MESSAGE("`kickoff_atom` has been send");
    }

    void run_server() {
        spawner_config cfg;
        spawner system {cfg.load<io::middleman>()};
        scoped_actor self {system};
        BOOST_TEST_MESSAGE("spawn peer acceptor");
        auto serv = system.middleman().spawn_broker(peer_acceptor_fun, system.spawn(pong));
        std::thread child;
        self->request(serv, infinite, publish_atom::value)
            .receive(
                [&](uint16_t port) {
                    BOOST_TEST_MESSAGE("server is running on port " << port);
                    child = std::thread([=] { run_client(port); });
                },
                [&](const error &err) { BOOST_ERROR("Error: " << self->system().render(err)); });
        self->await_all_other_actors_done();
        child.join();
    }

}    // namespace

BOOST_AUTO_TEST_CASE(test_broker_test) {
    run_server();
}
