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

#pragma once

#include <nil/mtl/config.hpp>

#include <thread>
#include <limits>
#include <memory>
#include <condition_variable>

#include <nil/mtl/detail/set_thread_name.hpp>
#include <nil/mtl/detail/thread_safe_actor_clock.hpp>
#include <nil/mtl/scheduler/abstract_coordinator.hpp>
#include <nil/mtl/scheduler/worker.hpp>

namespace nil {
    namespace mtl {
        namespace scheduler {

            /// Policy-based implementation of the abstract coordinator base class.
            template<class Policy>
            class coordinator : public abstract_coordinator {
            public:
                using super = abstract_coordinator;

                using policy_data = typename Policy::coordinator_data;

                coordinator(spawner &sys) : super(sys), data_(this) {
                    // nop
                }

                using worker_type = worker<Policy>;

                worker_type *worker_by_id(size_t x) {
                    return workers_[x].get();
                }

                policy_data &data() {
                    return data_;
                }

                static spawner::module *make(spawner &sys, detail::type_list<>) {
                    return new coordinator(sys);
                }

            protected:
                void start() override {
                    // Create initial state for all middleman_workers.
                    typename worker_type::policy_data init {this};
                    // Prepare middleman_workers vector.
                    auto num = num_workers();
                    workers_.reserve(num);
                    // Create worker instanes.
                    for (size_t i = 0; i < num; ++i)
                        workers_.emplace_back(new worker_type(i, this, init, max_throughput_));
                    // Start all middleman_workers.
                    for (auto &w : workers_)
                        w->start();
                    // Launch an additional background thread for dispatching timeouts and
                    // delayed messages.
                    timer_ = std::thread {[&] {
                        MTL_SET_LOGGER_SYS(&system());
                        detail::set_thread_name("mtl.clock");
                        system().thread_started();
                        clock_.run_dispatch_loop();
                        system().thread_terminates();
                    }};
                    // Run remaining startup code.
                    super::start();
                }

                void stop() override {
                    // shutdown middleman_workers
                    class shutdown_helper : public resumable, public ref_counted {
                    public:
                        resumable::resume_result resume(execution_unit *ptr, size_t) override {
                            MTL_ASSERT(ptr != nullptr);
                            std::unique_lock<std::mutex> guard(mtx);
                            last_worker = ptr;
                            cv.notify_all();
                            return resumable::shutdown_execution_unit;
                        }
                        void intrusive_ptr_add_ref_impl() override {
                            intrusive_ptr_add_ref(this);
                        }

                        void intrusive_ptr_release_impl() override {
                            intrusive_ptr_release(this);
                        }
                        shutdown_helper() : last_worker(nullptr) {
                            // nop
                        }
                        std::mutex mtx;
                        std::condition_variable cv;
                        execution_unit *last_worker;
                    };
                    // use a set to keep track of remaining middleman_workers
                    shutdown_helper sh;
                    std::set<worker_type *> alive_workers;
                    auto num = num_workers();
                    for (size_t i = 0; i < num; ++i) {
                        alive_workers.insert(worker_by_id(i));
                        sh.ref();    // make sure reference count is high enough
                    }
                    while (!alive_workers.empty()) {
                        (*alive_workers.begin())->external_enqueue(&sh);
                        // since jobs can be stolen, we cannot assume that we have
                        // actually shut down the worker we've enqueued sh to
                        {    // lifetime scope of guard
                            std::unique_lock<std::mutex> guard(sh.mtx);
                            sh.cv.wait(guard, [&] { return sh.last_worker != nullptr; });
                        }
                        alive_workers.erase(static_cast<worker_type *>(sh.last_worker));
                        sh.last_worker = nullptr;
                    }
                    // shutdown utility actors
                    stop_actors();
                    // wait until all middleman_workers are done
                    for (auto &w : workers_) {
                        w->get_thread().join();
                    }
                    // run cleanup code for each resumable
                    auto f = &abstract_coordinator::cleanup_and_release;
                    for (auto &w : workers_)
                        policy_.foreach_resumable(w.get(), f);
                    policy_.foreach_central_resumable(this, f);
                    // stop timer thread
                    clock_.cancel_dispatch_loop();
                    timer_.join();
                }

                void enqueue(resumable *ptr) override {
                    policy_.central_enqueue(this, ptr);
                }

                detail::thread_safe_actor_clock &clock() noexcept override {
                    return clock_;
                }

            private:
                /// System-wide clock.
                detail::thread_safe_actor_clock clock_;

                /// Set of workers.
                std::vector<std::unique_ptr<worker_type>> workers_;

                /// Policy-specific data.
                policy_data data_;

                /// The policy object.
                Policy policy_;

                /// Thread for managing timeouts and delayed messages.
                std::thread timer_;
            };

        }    // namespace scheduler
    }        // namespace mtl
}    // namespace nil
