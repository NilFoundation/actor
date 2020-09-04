//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/io/middleman.hpp>

#include <tuple>
#include <cerrno>
#include <memory>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include <nil/actor/sec.hpp>
#include <nil/actor/send.hpp>
#include <nil/actor/actor.hpp>
#include <nil/actor/after.hpp>
#include <nil/actor/config.hpp>
#include <nil/actor/logger.hpp>
#include <nil/actor/node_id.hpp>
#include <nil/actor/defaults.hpp>
#include <nil/actor/actor_proxy.hpp>
#include <nil/actor/make_counted.hpp>
#include <nil/actor/scoped_actor.hpp>
#include <nil/actor/function_view.hpp>
#include <nil/actor/actor_registry.hpp>
#include <nil/actor/event_based_actor.hpp>
#include <nil/actor/spawner_config.hpp>
#include <nil/actor/raw_event_based_actor.hpp>
#include <nil/actor/typed_event_based_actor.hpp>

#include <nil/actor/io/basp_broker.hpp>
#include <nil/actor/io/system_messages.hpp>

#include <nil/actor/io/network/interfaces.hpp>
#include <nil/actor/io/network/test_multiplexer.hpp>
#include <nil/actor/io/network/default_multiplexer.hpp>

#include <nil/actor/scheduler/abstract_coordinator.hpp>

#include <nil/actor/detail/safe_equal.hpp>
#include <nil/actor/detail/get_root_uuid.hpp>
#include <nil/actor/detail/set_thread_name.hpp>
#include <nil/actor/detail/get_mac_addresses.hpp>

#ifdef BOOST_OS_WINDOWS_AVAILABLE
#include <io.h>
#include <fcntl.h>
#endif    // BOOST_OS_WINDOWS_AVAILABLE

namespace nil {
    namespace actor {
        namespace io {

            namespace {

                template<class T>
                class mm_impl : public middleman {
                public:
                    mm_impl(spawner &ref) : middleman(ref), backend_(&ref) {
                        // nop
                    }

                    network::multiplexer &backend() override {
                        return backend_;
                    }

                private:
                    T backend_;
                };

            }    // namespace

            spawner_module *middleman::make(spawner &sys, detail::type_list<>) {
                auto atm = sys.config().middleman_network_backend;
                switch (atom_uint(atm)) {
                    case atom_uint(atom("testing")):
                        return new mm_impl<network::test_multiplexer>(sys);
                    default:
                        return new mm_impl<network::default_multiplexer>(sys);
                }
            }

            middleman::middleman(spawner &sys) : system_(sys) {
                // nop
            }

            expected<strong_actor_ptr> middleman::remote_spawn_impl(const node_id &nid,
                                                                    std::string &name,
                                                                    message &args,
                                                                    std::set<std::string>
                                                                        s,
                                                                    duration timeout) {
                auto f = make_function_view(actor_handle(), timeout);
                return f(spawn_atom::value, nid, std::move(name), std::move(args), std::move(s));
            }

            expected<uint16_t> middleman::open(uint16_t port, const char *in, bool reuse) {
                std::string str;
                if (in != nullptr)
                    str = in;
                auto f = make_function_view(actor_handle());
                return f(open_atom::value, port, std::move(str), reuse);
            }

            expected<void> middleman::close(uint16_t port) {
                auto f = make_function_view(actor_handle());
                return f(close_atom::value, port);
            }

            expected<node_id> middleman::connect(std::string host, uint16_t port) {
                auto f = make_function_view(actor_handle());
                auto res = f(connect_atom::value, std::move(host), port);
                if (!res)
                    return std::move(res.error());
                return std::get<0>(*res);
            }

            expected<uint16_t> middleman::publish(const strong_actor_ptr &whom, std::set<std::string> sigs,
                                                  uint16_t port, const char *cstr, bool ru) {
                ACTOR_LOG_TRACE(ACTOR_ARG(whom) << ACTOR_ARG(sigs) << ACTOR_ARG(port));
                if (!whom)
                    return sec::cannot_publish_invalid_actor;
                std::string in;
                if (cstr != nullptr)
                    in = cstr;
                auto f = make_function_view(actor_handle());
                return f(publish_atom::value, port, std::move(whom), std::move(sigs), in, ru);
            }

            expected<uint16_t> middleman::publish_local_groups(uint16_t port, const char *in, bool reuse) {
                ACTOR_LOG_TRACE(ACTOR_ARG(port) << ACTOR_ARG(in));
                auto group_nameserver = [](event_based_actor *self) -> behavior {
                    return {
                        [self](get_atom, const std::string &name) { return self->system().groups().get_local(name); }};
                };
                auto gn = system().spawn<hidden + lazy_init>(group_nameserver);
                auto result = publish(gn, port, in, reuse);
                // link gn to our manager
                if (result)
                    manager_->add_link(actor_cast<abstract_actor *>(gn));
                else
                    anon_send_exit(gn, exit_reason::user_shutdown);
                return result;
            }

            expected<void> middleman::unpublish(const actor_addr &whom, uint16_t port) {
                ACTOR_LOG_TRACE(ACTOR_ARG(whom) << ACTOR_ARG(port));
                auto f = make_function_view(actor_handle());
                return f(unpublish_atom::value, whom, port);
            }

            expected<strong_actor_ptr> middleman::remote_actor(std::set<std::string> ifs, std::string host,
                                                               uint16_t port) {
                ACTOR_LOG_TRACE(ACTOR_ARG(ifs) << ACTOR_ARG(host) << ACTOR_ARG(port));
                auto f = make_function_view(actor_handle());
                auto res = f(connect_atom::value, std::move(host), port);
                if (!res)
                    return std::move(res.error());
                strong_actor_ptr ptr = std::move(std::get<1>(*res));
                if (!ptr)
                    return make_error(sec::no_actor_published_at_port, port);
                if (!system().assignable(std::get<2>(*res), ifs))
                    return make_error(sec::unexpected_actor_messaging_interface, std::move(ifs),
                                      std::move(std::get<2>(*res)));
                return ptr;
            }

            expected<group> middleman::remote_group(const std::string &group_uri) {
                ACTOR_LOG_TRACE(ACTOR_ARG(group_uri));
                // format of group_identifier is group@host:port
                // a regex would be the natural choice here, but we want to support
                // older compilers that don't have <regex> implemented (e.g. GCC < 4.9)
                auto pos1 = group_uri.find('@');
                auto pos2 = group_uri.find(':');
                auto last = std::string::npos;
                if (pos1 == last || pos2 == last || pos1 >= pos2)
                    return make_error(sec::invalid_argument, "invalid URI format", group_uri);
                auto name = group_uri.substr(0, pos1);
                auto host = group_uri.substr(pos1 + 1, pos2 - pos1 - 1);
                auto port = static_cast<uint16_t>(std::stoi(group_uri.substr(pos2 + 1)));
                return remote_group(name, host, port);
            }

            expected<group> middleman::remote_group(const std::string &group_identifier, const std::string &host,
                                                    uint16_t port) {
                ACTOR_LOG_TRACE(ACTOR_ARG(group_identifier) << ACTOR_ARG(host) << ACTOR_ARG(port));
                // Helper actor that first connects to the remote actor at `host:port` and
                // then tries to get a valid group from that actor.
                auto two_step_lookup = [=](event_based_actor *self, middleman_actor mm) -> behavior {
                    return {[=](get_atom) {
                        /// We won't receive a second message, so we drop our behavior here to
                        /// terminate the actor after both requests finish.
                        self->unbecome();
                        auto rp = self->make_response_promise();
                        self->request(mm, infinite, connect_atom::value, host, port)
                            .then([=](const node_id &, strong_actor_ptr &ptr, const std::set<std::string> &) mutable {
                                auto hdl = actor_cast<actor>(ptr);
                                self->request(hdl, infinite, get_atom::value, group_identifier)
                                    .then([=](group &result) mutable { rp.deliver(std::move(result)); });
                            });
                    }};
                };
                // Spawn the helper actor and wait for the result.
                expected<group> result {sec::cannot_connect_to_node};
                scoped_actor self {system(), true};
                self->request(self->spawn<lazy_init>(two_step_lookup, actor_handle()), infinite, get_atom::value)
                    .receive([&](group &grp) { result = std::move(grp); },
                             [&](error &err) { result = std::move(err); });
                return result;
            }

            strong_actor_ptr middleman::remote_lookup(atom_value name, const node_id &nid) {
                ACTOR_LOG_TRACE(ACTOR_ARG(name) << ACTOR_ARG(nid));
                if (system().node() == nid)
                    return system().registry().get(name);
                auto basp = named_broker<basp_broker>(atom("BASP"));
                strong_actor_ptr result;
                scoped_actor self {system(), true};
                self->send(basp, forward_atom::value, nid, atom("ConfigServ"), make_message(get_atom::value, name));
                self->receive([&](strong_actor_ptr &addr) { result = std::move(addr); },
                              after(std::chrono::minutes(5)) >>
                                  [] {
                                      // nop
                                  });
                return result;
            }

            void middleman::start() {
                ACTOR_LOG_TRACE("");
                // Launch backend.
                if (!config().middleman_manual_multiplexing)
                    backend_supervisor_ = backend().make_supervisor();
                // The only backend that returns a `nullptr` by default is the
                // `test_multiplexer` which does not have its own thread but uses the main
                // thread instead. Other backends can set `middleman_detach_multiplexer` to
                // false to suppress creation of the supervisor.
                if (backend_supervisor_ != nullptr) {
                    std::atomic<bool> init_done {false};
                    std::mutex mtx;
                    std::condition_variable cv;
                    thread_ = std::thread {[&, this] {
                        ACTOR_SET_LOGGER_SYS(&system());
                        detail::set_thread_name("actor.multiplexer");
                        system().thread_started();
                        ACTOR_LOG_TRACE("");
                        {
                            std::unique_lock<std::mutex> guard {mtx};
                            backend().thread_id(std::this_thread::get_id());
                            init_done = true;
                            cv.notify_one();
                        }
                        backend().run();
                        system().thread_terminates();
                    }};
                    std::unique_lock<std::mutex> guard {mtx};
                    while (init_done == false)
                        cv.wait(guard);
                }
                // Spawn utility actors.
                auto basp = named_broker<basp_broker>(atom("BASP"));
                manager_ = make_middleman_actor(system(), basp);
            }

            void middleman::stop() {
                ACTOR_LOG_TRACE("");
                backend().dispatch([=] {
                    ACTOR_LOG_TRACE("");
                    // managers_ will be modified while we are stopping each manager,
                    // because each manager will call remove(...)
                    for (auto &kvp : named_brokers_) {
                        auto &hdl = kvp.second;
                        auto ptr = static_cast<broker *>(actor_cast<abstract_actor *>(hdl));
                        if (!ptr->getf(abstract_actor::is_terminated_flag)) {
                            ptr->context(&backend());
                            ptr->quit();
                            ptr->finalize();
                        }
                    }
                });
                if (!config().middleman_manual_multiplexing) {
                    backend_supervisor_.reset();
                    if (thread_.joinable())
                        thread_.join();
                } else {
                    while (backend().try_run_once())
                        ;    // nop
                }
                named_brokers_.clear();
                scoped_actor self {system(), true};
                self->send_exit(manager_, exit_reason::kill);
                if (!config().middleman_attach_utility_actors)
                    self->wait_for(manager_);
                destroy(manager_);
            }

            void middleman::init(spawner_config &cfg) {
                // never detach actors when using the testing multiplexer
                auto network_backend = cfg.middleman_network_backend;
                if (network_backend == atom("testing")) {
                    cfg.middleman_attach_utility_actors = true;
                    cfg.middleman_manual_multiplexing = true;
                }
                // add remote group module to config
                struct remote_groups : group_module {
                public:
                    remote_groups(middleman &parent) : group_module(parent.system(), "remote"), parent_(parent) {
                        // nop
                    }

                    void stop() override {
                        // nop
                    }

                    expected<group> get(const std::string &group_name) override {
                        return parent_.remote_group(group_name);
                    }

                    error load(deserializer &, group &) override {
                        // never called, because we hand out group instances of the local module
                        return sec::no_such_group_module;
                    }

                    error_code<sec> load(binary_deserializer &, group &) override {
                        // never called, because we hand out group instances of the local module
                        return sec::no_such_group_module;
                    }

                private:
                    middleman &parent_;
                };
                auto gfactory = [=]() -> group_module * { return new remote_groups(*this); };
                cfg.group_module_factories.emplace_back(gfactory);
                // logging not available at this stage
                // add I/O-related types to config
                cfg.add_message_type<network::protocol>("@protocol")
                    .add_message_type<network::address_listing>("@address_listing")
                    .add_message_type<network::receive_buffer>("@receive_buffer")
                    .add_message_type<new_data_msg>("@new_data_msg")
                    .add_message_type<new_connection_msg>("@new_connection_msg")
                    .add_message_type<acceptor_closed_msg>("@acceptor_closed_msg")
                    .add_message_type<connection_closed_msg>("@connection_closed_msg")
                    .add_message_type<accept_handle>("@accept_handle")
                    .add_message_type<connection_handle>("@connection_handle")
                    .add_message_type<connection_passivated_msg>("@connection_passivated_msg")
                    .add_message_type<acceptor_passivated_msg>("@acceptor_passivated_msg");
                // Compute and set ID for this network node.
                auto this_node = node_id::default_data::local(cfg);
                system().node_.swap(this_node);
                // give config access to slave mode implementation
                cfg.slave_mode_fun = &middleman::exec_slave_mode;
            }

            spawner_module::id_t middleman::id() const {
                return module::middleman;
            }

            void *middleman::subtype_ptr() {
                return this;
            }

            middleman::~middleman() {
                // nop
            }

            middleman_actor middleman::actor_handle() {
                return manager_;
            }

            int middleman::exec_slave_mode(spawner &, const spawner_config &) {
                // TODO
                return 0;
            }

        }    // namespace io
    }        // namespace actor
}    // namespace nil
