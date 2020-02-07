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

#include <nil/mtl/spawner.hpp>

#include <memory>
#include <unordered_set>

#include <nil/mtl/spawner_config.hpp>
#include <nil/mtl/event_based_actor.hpp>
#include <nil/mtl/raise_error.hpp>
#include <nil/mtl/raw_event_based_actor.hpp>
#include <nil/mtl/send.hpp>
#include <nil/mtl/to_string.hpp>

#include <nil/mtl/policy/work_sharing.hpp>
#include <nil/mtl/policy/work_stealing.hpp>

#include <nil/mtl/scheduler/coordinator.hpp>
#include <nil/mtl/scheduler/test_coordinator.hpp>
#include <nil/mtl/scheduler/abstract_coordinator.hpp>
#include <nil/mtl/scheduler/profiled_coordinator.hpp>

namespace nil {
    namespace mtl {

        namespace {

            struct kvstate {
                using key_type = std::string;
                using mapped_type = message;
                using subscriber_set = std::unordered_set<strong_actor_ptr>;
                using topic_set = std::unordered_set<std::string>;
                std::unordered_map<key_type, std::pair<mapped_type, subscriber_set>> data;
                std::unordered_map<strong_actor_ptr, topic_set> subscribers;
                static const char *name;
            };

            const char *kvstate::name = "config_server";

            behavior config_serv_impl(stateful_actor<kvstate> *self) {
                MTL_LOG_TRACE("");
                std::string wildcard = "*";
                auto unsubscribe_all = [=](actor subscriber) {
                    auto &subscribers = self->state.subscribers;
                    auto ptr = actor_cast<strong_actor_ptr>(subscriber);
                    auto i = subscribers.find(ptr);
                    if (i == subscribers.end())
                        return;
                    for (auto &key : i->second)
                        self->state.data[key].second.erase(ptr);
                    subscribers.erase(i);
                };
                self->set_down_handler([=](down_msg &dm) {
                    MTL_LOG_TRACE(MTL_ARG(dm));
                    auto ptr = actor_cast<strong_actor_ptr>(dm.source);
                    if (ptr)
                        unsubscribe_all(actor_cast<actor>(std::move(ptr)));
                });
                return {// set a key/value pair
                        [=](put_atom, const std::string &key, message &msg) {
                            MTL_LOG_TRACE(MTL_ARG(key) << MTL_ARG(msg));
                            if (key == "*")
                                return;
                            auto &vp = self->state.data[key];
                            vp.first = std::move(msg);
                            for (auto &subscriber_ptr : vp.second) {
                                // we never put a nullptr in our map
                                auto subscriber = actor_cast<actor>(subscriber_ptr);
                                if (subscriber != self->current_sender())
                                    self->send(subscriber, update_atom::value, key, vp.first);
                            }
                            // also iterate all subscribers for '*'
                            for (auto &subscriber : self->state.data[wildcard].second)
                                if (subscriber != self->current_sender())
                                    self->send(actor_cast<actor>(subscriber), update_atom::value, key, vp.first);
                        },
                        // get a key/value pair
                        [=](get_atom, std::string &key) -> message {
                            MTL_LOG_TRACE(MTL_ARG(key));
                            if (key == wildcard) {
                                std::vector<std::pair<std::string, message>> msgs;
                                for (auto &kvp : self->state.data)
                                    if (kvp.first != "*")
                                        msgs.emplace_back(kvp.first, kvp.second.first);
                                return make_message(std::move(msgs));
                            }
                            auto i = self->state.data.find(key);
                            return make_message(std::move(key),
                                                i != self->state.data.end() ? i->second.first : make_message());
                        },
                        // subscribe to a key
                        [=](subscribe_atom, const std::string &key) {
                            auto subscriber = actor_cast<strong_actor_ptr>(self->current_sender());
                            MTL_LOG_TRACE(MTL_ARG(key) << MTL_ARG(subscriber));
                            if (!subscriber)
                                return;
                            self->state.data[key].second.insert(subscriber);
                            auto &subscribers = self->state.subscribers;
                            auto i = subscribers.find(subscriber);
                            if (i != subscribers.end()) {
                                i->second.insert(key);
                            } else {
                                self->monitor(subscriber);
                                subscribers.emplace(subscriber, kvstate::topic_set {key});
                            }
                        },
                        // unsubscribe from a key
                        [=](unsubscribe_atom, const std::string &key) {
                            auto subscriber = actor_cast<strong_actor_ptr>(self->current_sender());
                            if (!subscriber)
                                return;
                            MTL_LOG_TRACE(MTL_ARG(key) << MTL_ARG(subscriber));
                            if (key == wildcard) {
                                unsubscribe_all(actor_cast<actor>(std::move(subscriber)));
                                return;
                            }
                            self->state.subscribers[subscriber].erase(key);
                            self->state.data[key].second.erase(subscriber);
                        },
                        // get a 'named' actor from local registry
                        [=](get_atom, atom_value name) { return self->home_system().registry().get(name); }};
            }

            // -- spawn server -------------------------------------------------------------

            // A spawn server allows users to spawn actors dynamically with a name and a
            // message containing the data for initialization. By accessing the spawn server
            // on another node, users can spwan actors remotely.

            struct spawn_serv_state {
                static const char *name;
            };

            const char *spawn_serv_state::name = "spawn_server";

            behavior spawn_serv_impl(stateful_actor<spawn_serv_state> *self) {
                MTL_LOG_TRACE("");
                return {[=](spawn_atom, const std::string &name, message &args,
                            actor_system::mpi &xs) -> expected<strong_actor_ptr> {
                    MTL_LOG_TRACE(MTL_ARG(name) << MTL_ARG(args));
                    return self->system().spawn<strong_actor_ptr>(name, std::move(args), self->context(), true, &xs);
                }};
            }

            // -- stream server ------------------------------------------------------------

            // The stream server acts as a man-in-the-middle for all streams that cross the
            // network. It manages any number of unrelated streams by placing itself and the
            // stream server on the next remote node into the pipeline.

            // Outgoing messages are buffered in FIFO order to ensure fairness. However, the
            // stream server uses five different FIFO queues: on for each priority level.
            // A high priority grants more network bandwidth.

            // Note that stream servers do not actively take part in the streams they
            // process. Batch messages and ACKs are treated equally. Open, close, and error
            // messages are evaluated to add and remove state as needed.

            class dropping_execution_unit : public execution_unit {
            public:
                dropping_execution_unit(actor_system *sys) : execution_unit(sys) {
                    // nop
                }

                void exec_later(resumable *) override {
                    // should not happen in the first place
                    MTL_LOG_ERROR("actor registry actor called exec_later during shutdown");
                }
            };

        }    // namespace

        actor_system::module::~module() {
            // nop
        }

        const char *actor_system::module::name() const noexcept {
            switch (id()) {
                case scheduler:
                    return "Scheduler";
                case middleman:
                    return "Middleman";
                case opencl_manager:
                    return "OpenCL Manager";
                case openssl_manager:
                    return "OpenSSL Manager";
                default:
                    return "???";
            }
        }

        actor_system::spawner(actor_system_config &cfg) :
            ids_(0), types_(*this), logger_(new nil::mtl::logger(*this), false), registry_(*this), groups_(*this),
            dummy_execution_unit_(this), await_actors_before_shutdown_(true), detached_(0), cfg_(cfg),
            logger_dtor_done_(false) {
            MTL_SET_LOGGER_SYS(this);
            for (auto &hook : cfg.thread_hooks_)
                hook->init(*this);
            for (auto &f : cfg.module_factories) {
                auto mod_ptr = f(*this);
                modules_[mod_ptr->id()].reset(mod_ptr);
            }
            auto &sched = modules_[module::scheduler];
            using namespace scheduler;
            using policy::work_sharing;
            using policy::work_stealing;
            using share = coordinator<work_sharing>;
            using steal = coordinator<work_stealing>;
            using profiled_share = profiled_coordinator<policy::profiled<work_sharing>>;
            using profiled_steal = profiled_coordinator<policy::profiled<work_stealing>>;
            // set scheduler only if not explicitly loaded by user
            if (!sched) {
                enum sched_conf {
                    stealing = 0x0001,
                    sharing = 0x0002,
                    testing = 0x0003,
                    profiled = 0x0100,
                    profiled_stealing = 0x0101,
                    profiled_sharing = 0x0102
                };
                sched_conf sc = stealing;
                namespace sr = defaults::scheduler;
                auto sr_policy = cfg.scheduler_policy;
                if (sr_policy == atom("sharing"))
                    sc = sharing;
                else if (sr_policy == atom("testing"))
                    sc = testing;
                else if (sr_policy != atom("stealing"))
                    std::cerr << "[WARNING] " << deep_to_string(sr_policy)
                              << " is an unrecognized scheduler pollicy, "
                                 "falling back to 'stealing' (i.e. work-stealing)"
                              << std::endl;
                if (cfg.scheduler_enable_profiling)
                    sc = static_cast<sched_conf>(sc | profiled);
                switch (sc) {
                    default:    // any invalid configuration falls back to work stealing
                        sched = std::make_unique<steal>(*this);
                        break;
                    case sharing:
                        sched = std::make_unique<share>(*this);
                        break;
                    case profiled_stealing:
                        sched = std::make_unique<profiled_steal>(*this);
                        break;
                    case profiled_sharing:
                        sched = std::make_unique<profiled_share>(*this);
                        break;
                    case testing:
                        sched = std::make_unique<test_coordinator>(*this);
                }
            }
            // initialize state for each module and give each module the opportunity
            // to influence the system configuration, e.g., by adding more types
            logger_->init(cfg);
            MTL_SET_LOGGER_SYS(this);
            for (auto &mod : modules_) {
                if (mod) {
                    mod->init(cfg);
                }
            }
            groups_.init(cfg);
            // spawn config and spawn servers (lazily to not access the scheduler yet)
            static constexpr auto Flags = hidden + lazy_init;
            spawn_serv(actor_cast<strong_actor_ptr>(spawn<Flags>(spawn_serv_impl)));
            config_serv(actor_cast<strong_actor_ptr>(spawn<Flags>(config_serv_impl)));
            // fire up remaining modules
            registry_.start();
            registry_.put(atom("SpawnServ"), spawn_serv());
            registry_.put(atom("ConfigServ"), config_serv());
            for (auto &mod : modules_)
                if (mod)
                    mod->start();
            groups_.start();
            logger_->start();
        }

        actor_system::~spawner() {
            {
                MTL_LOG_TRACE("");
                MTL_LOG_DEBUG("shutdown actor system");
                if (await_actors_before_shutdown_)
                    await_all_actors_done();
                // shutdown internal actors
                for (auto &x : internal_actors_) {
                    anon_send_exit(x, exit_reason::user_shutdown);
                    x = nullptr;
                }
                registry_.erase(atom("SpawnServ"));
                registry_.erase(atom("ConfigServ"));
                registry_.erase(atom("StreamServ"));
                // group module is the first one, relies on MM
                groups_.stop();
                // stop modules in reverse order
                for (auto i = modules_.rbegin(); i != modules_.rend(); ++i) {
                    auto &ptr = *i;
                    if (ptr != nullptr) {
                        MTL_LOG_DEBUG("stop module" << ptr->name());
                        ptr->stop();
                    }
                }
                await_detached_threads();
                registry_.stop();
            }
            // reset logger and wait until dtor was called
            MTL_SET_LOGGER_SYS(nullptr);
            logger_.reset();
            std::unique_lock<std::mutex> guard {logger_dtor_mtx_};
            while (!logger_dtor_done_)
                logger_dtor_cv_.wait(guard);
        }

        /// Returns the host-local identifier for this system.
        const node_id &actor_system::node() const {
            return node_;
        }

        /// Returns the scheduler instance.
        scheduler::abstract_coordinator &actor_system::scheduler() {
            using ptr = scheduler::abstract_coordinator *;
            return *dynamic_cast<ptr>(modules_[module::scheduler].get());
        }

        nil::mtl::logger &actor_system::logger() {
            return *logger_;
        }

        actor_registry &actor_system::registry() {
            return registry_;
        }

        const uniform_type_info_map &actor_system::types() const {
            return types_;
        }

        std::string actor_system::render(const error &x) const {
            if (!x)
                return to_string(x);
            auto &xs = config().error_renderers;
            auto i = xs.find(x.category());
            if (i != xs.end())
                return i->second(x.code(), x.category(), x.context());
            return to_string(x);
        }

        group_manager &actor_system::groups() {
            return groups_;
        }

        bool actor_system::has_middleman() const {
            return modules_[module::middleman] != nullptr;
        }

        io::middleman &actor_system::middleman() {
            auto &clptr = modules_[module::middleman];
            if (!clptr)
                MTL_RAISE_ERROR("cannot access middleman: module not loaded");
            return *reinterpret_cast<io::middleman *>(clptr->subtype_ptr());
        }

        bool actor_system::has_opencl_manager() const {
            return modules_[module::opencl_manager] != nullptr;
        }

        opencl::manager &actor_system::opencl_manager() const {
            auto &clptr = modules_[module::opencl_manager];
            if (!clptr)
                MTL_RAISE_ERROR("cannot access opencl manager: module not loaded");
            return *reinterpret_cast<opencl::manager *>(clptr->subtype_ptr());
        }

        bool actor_system::has_openssl_manager() const {
            return modules_[module::openssl_manager] != nullptr;
        }

        openssl::manager &actor_system::openssl_manager() const {
            auto &clptr = modules_[module::openssl_manager];
            if (!clptr)
                MTL_RAISE_ERROR("cannot access openssl manager: module not loaded");
            return *reinterpret_cast<openssl::manager *>(clptr->subtype_ptr());
        }

        scoped_execution_unit *actor_system::dummy_execution_unit() {
            return &dummy_execution_unit_;
        }

        actor_id actor_system::next_actor_id() {
            return ++ids_;
        }

        actor_id actor_system::latest_actor_id() const {
            return ids_.load();
        }

        void actor_system::await_all_actors_done() const {
            registry_.await_running_count_equal(0);
        }

        actor_clock &actor_system::clock() noexcept {
            return scheduler().clock();
        }

        void actor_system::inc_detached_threads() {
            ++detached_;
        }

        void actor_system::dec_detached_threads() {
            std::unique_lock<std::mutex> guard {detached_mtx_};
            if (--detached_ == 0)
                detached_cv_.notify_all();
        }

        void actor_system::await_detached_threads() {
            std::unique_lock<std::mutex> guard {detached_mtx_};
            while (detached_ != 0)
                detached_cv_.wait(guard);
        }

        void actor_system::thread_started() {
            for (auto &hook : cfg_.thread_hooks_)
                hook->thread_started();
        }

        void actor_system::thread_terminates() {
            for (auto &hook : cfg_.thread_hooks_)
                hook->thread_terminates();
        }

        expected<strong_actor_ptr> actor_system::dyn_spawn_impl(const std::string &name, message &args,
                                                                execution_unit *ctx, bool check_interface,
                                                                optional<const mpi &> expected_ifs) {
            MTL_LOG_TRACE(MTL_ARG(name) << MTL_ARG(args) << MTL_ARG(check_interface) << MTL_ARG(expected_ifs));
            if (name.empty())
                return sec::invalid_argument;
            auto &fs = cfg_.actor_factories;
            auto i = fs.find(name);
            if (i == fs.end())
                return sec::unknown_type;
            actor_config cfg {ctx != nullptr ? ctx : &dummy_execution_unit_};
            auto res = i->second(cfg, args);
            if (!res.first)
                return sec::cannot_spawn_actor_from_arguments;
            if (check_interface && !assignable(res.second, *expected_ifs))
                return sec::unexpected_actor_messaging_interface;
            return std::move(res.first);
        }

    }    // namespace mtl
}    // namespace nil
