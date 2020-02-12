#pragma once

#include <tuple>
#include <vector>
#include <numeric>
#include <algorithm>
#include <functional>

#include <nil/actor/abstract_actor.hpp>
#include <nil/actor/actor_cast.hpp>
#include <nil/actor/logger.hpp>
#include <nil/actor/raise_error.hpp>
#include <nil/actor/response_promise.hpp>

#include <nil/actor/detail/raw_ptr.hpp>
#include <nil/actor/detail/scope_guard.hpp>

#include <nil/actor/opencl/global.hpp>
#include <nil/actor/opencl/nd_range.hpp>
#include <nil/actor/opencl/arguments.hpp>
#include <nil/actor/opencl/opencl_err.hpp>

namespace nil {
    namespace actor {
        namespace opencl {

            /// A command represents the execution of a kernel on a device. It handles the
            /// OpenCL calls to enqueue the kernel with the index space and keeps references
            /// to the management data during the execution. Furthermore, the command sends
            /// the execution results to the responsible actor.
            template<class Actor, class... Ts>
            class command : public ref_counted {
            public:
                using result_types = detail::type_list<Ts...>;

                command(response_promise promise, strong_actor_ptr parent, std::vector<cl_event> events,
                        std::vector<detail::raw_mem_ptr> inputs, std::vector<detail::raw_mem_ptr> outputs,
                        std::vector<detail::raw_mem_ptr> scratches, std::vector<size_t> lengths, message msg,
                        std::tuple<Ts...> output_tuple, nd_range range) :
                    lengths_(std::move(lengths)),
                    promise_(std::move(promise)), cl_actor_(std::move(parent)), mem_in_events_(std::move(events)),
                    input_buffers_(std::move(inputs)), output_buffers_(std::move(outputs)),
                    scratch_buffers_(std::move(scratches)), results_(std::move(output_tuple)), msg_(std::move(msg)),
                    range_(std::move(range)) {
                    // nop
                }

                ~command() override {
                    for (auto &e : mem_in_events_) {
                        if (e) {
                            v1callcl(ACTOR_CLF(clReleaseEvent), e);
                        }
                    }
                    for (auto &e : mem_out_events_) {
                        if (e) {
                            v1callcl(ACTOR_CLF(clReleaseEvent), e);
                        }
                    }
                }

                /// Enqueue the kernel for execution, schedule reading of the results and
                /// set a callback to send the results to the actor identified by the handle.
                /// Only called if the results includes at least one type that is not a
                /// mem_ref.
                template<class Q = result_types>
                typename std::enable_if<!detail::tl_forall<Q, is_ref_type>::value>::type enqueue() {
                    // Errors in this function can not be handled by opencl_err.hpp
                    // because they require non-standard error handling
                    ACTOR_LOG_TRACE("");
                    this->ref();    // reference held by the OpenCL comand queue
                    auto data_or_nullptr = [](const dim_vec &vec) { return vec.empty() ? nullptr : vec.data(); };
                    auto parent = static_cast<Actor *>(actor_cast<abstract_actor *>(cl_actor_));
                    // OpenCL expects cl_uint (unsigned int), hence the cast
                    mem_out_events_.emplace_back();
                    auto success = invoke_cl(
                        clEnqueueNDRangeKernel, parent->queue_.get(), parent->kernel_.get(),
                        static_cast<unsigned int>(range_.dimensions().size()), data_or_nullptr(range_.offsets()),
                        data_or_nullptr(range_.dimensions()), data_or_nullptr(range_.local_dimensions()),
                        static_cast<unsigned int>(mem_in_events_.size()),
                        (mem_in_events_.empty() ? nullptr : mem_in_events_.data()), &mem_out_events_.back());
                    if (!success) {
                        return;
                    }
                    size_t pos = 0;
                    ACTOR_ASSERT(!mem_out_events_.empty());
                    enqueue_read_buffers(pos, mem_out_events_, detail::get_indices(results_));
                    cl_event marker_event;
#if defined(__APPLE__)
                    success = invoke_cl(clEnqueueMarkerWithWaitList, parent->queue_.get(),
                                        static_cast<unsigned int>(mem_out_events_.size()), mem_out_events_.data(),
                                        &marker_event);
#else
                    success = invoke_cl(clEnqueueMarker, parent->queue_.get(), &marker_event);
#endif
                    callback_.reset(marker_event, false);
                    if (!success) {
                        return;
                    }
                    auto cb = [](cl_event, cl_int, void *data) {
                        auto cmd = reinterpret_cast<command *>(data);
                        cmd->handle_results();
                        cmd->deref();
                    };
                    if (!invoke_cl(clSetEventCallback, callback_.get(), CL_COMPLETE, std::move(cb), this)) {
                        return;
                    }
                    v3callcl(clFlush, parent->queue_.get());
                }

                /// Enqueue the kernel for execution and send the mem_refs relating to the
                /// results to the next actor. A callback is set to clean up the commmand
                /// once the execution is finished. Only called if the results only consist
                /// of mem_ref types.
                template<class Q = result_types>
                typename std::enable_if<detail::tl_forall<Q, is_ref_type>::value>::type enqueue() {
                    // Errors in this function can not be handled by opencl_err.hpp
                    // because they require non-standard error handling
                    ACTOR_LOG_TRACE("");
                    this->ref();    // reference held by the OpenCL command queue
                    auto data_or_nullptr = [](const dim_vec &vec) { return vec.empty() ? nullptr : vec.data(); };
                    auto parent = static_cast<Actor *>(actor_cast<abstract_actor *>(cl_actor_));
                    cl_event execution_event;
                    auto success =
                        invoke_cl(clEnqueueNDRangeKernel, parent->queue_.get(), parent->kernel_.get(),
                                  static_cast<cl_uint>(range_.dimensions().size()), data_or_nullptr(range_.offsets()),
                                  data_or_nullptr(range_.dimensions()), data_or_nullptr(range_.local_dimensions()),
                                  static_cast<unsigned int>(mem_in_events_.size()),
                                  (mem_in_events_.empty() ? nullptr : mem_in_events_.data()), &execution_event);
                    callback_.reset(execution_event, false);
                    if (!success) {
                        return;
                    }
                    auto cb = [](cl_event, cl_int, void *data) {
                        auto c = reinterpret_cast<command *>(data);
                        c->deref();
                    };
                    if (!invoke_cl(clSetEventCallback, callback_.get(), CL_COMPLETE, std::move(cb), this)) {
                        return;
                    }
                    v3callcl(clFlush, parent->queue_.get());
                    auto msg = msg_adding_event {callback_}(results_);
                    promise_.deliver(std::move(msg));
                }

            private:
                template<long I, class T>
                void enqueue_read(std::vector<T> &, std::vector<cl_event> &events, size_t &pos) {
                    auto p = static_cast<Actor *>(actor_cast<abstract_actor *>(cl_actor_));
                    events.emplace_back();
                    auto size = lengths_[pos];
                    auto buffer_size = sizeof(T) * size;
                    std::get<I>(results_).resize(size);
                    auto err =
                        clEnqueueReadBuffer(p->queue_.get(), output_buffers_[pos].get(), CL_FALSE, 0, buffer_size,
                                            std::get<I>(results_).data(), 1, events.data(), &events.back());
                    if (err != CL_SUCCESS) {
                        this->deref();    // failed to enqueue command
                        ACTOR_RAISE_ERROR("failed to enqueue command");
                    }
                    pos += 1;
                }

                template<long I, class T>
                void enqueue_read(mem_ref<T> &, std::vector<cl_event> &, size_t &) {
                    // Nothing to read back if we return references.
                }

                void enqueue_read_buffers(size_t &, std::vector<cl_event> &, detail::int_list<>) {
                    // end of recursion
                }

                template<long I, long... Is>
                void enqueue_read_buffers(size_t &pos, std::vector<cl_event> &events, detail::int_list<I, Is...>) {
                    enqueue_read<I>(std::get<I>(results_), events, pos);
                    enqueue_read_buffers(pos, events, detail::int_list<Is...> {});
                }

                // handle results if execution result includes a value type
                void handle_results() {
                    auto parent = static_cast<Actor *>(actor_cast<abstract_actor *>(cl_actor_));
                    auto &map_fun = parent->map_results_;
                    auto msg = map_fun ? apply_args(map_fun, detail::get_indices(results_), results_) :
                                         message_from_results {}(results_);
                    promise_.deliver(std::move(msg));
                }

                // call function F and derefenrence the command on failure
                template<class F, class... Us>
                bool invoke_cl(F f, Us &&... xs) {
                    auto err = f(std::forward<Us>(xs)...);
                    if (err == CL_SUCCESS) {
                        return true;
                    }
                    ACTOR_LOG_ERROR("error: " << opencl_error(err));
                    this->deref();
                    return false;
                }

                std::vector<size_t> lengths_;
                response_promise promise_;
                strong_actor_ptr cl_actor_;
                std::vector<cl_event> mem_in_events_;
                std::vector<cl_event> mem_out_events_;
                detail::raw_event_ptr callback_;
                std::vector<detail::raw_mem_ptr> input_buffers_;
                std::vector<detail::raw_mem_ptr> output_buffers_;
                std::vector<detail::raw_mem_ptr> scratch_buffers_;
                std::tuple<Ts...> results_;
                message msg_;    // keeps the argument buffers alive for async copy to device
                nd_range range_;
            };

        }    // namespace opencl
    }        // namespace actor
}    // namespace nil