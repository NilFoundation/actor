#pragma once

#include <map>
#include <memory>

#include <nil/actor/ref_counted.hpp>

#include <nil/actor/detail/raw_ptr.hpp>

#include <nil/actor/opencl/device.hpp>
#include <nil/actor/opencl/global.hpp>

namespace nil {
    namespace actor {
        namespace opencl {

            class program;

            using program_ptr = intrusive_ptr<program>;

            /// @brief A wrapper for OpenCL's cl_program.
            class program : public ref_counted {
            public:
                friend class manager;

                template<bool PassConfig, class... Ts>
                friend class actor_facade;

                template<class T, class... Ts>
                friend intrusive_ptr<T> nil::actor::make_counted(Ts &&...);

            private:
                program(detail::raw_context_ptr context, detail::raw_command_queue_ptr queue,
                        detail::raw_program_ptr prog, std::map<std::string, detail::raw_kernel_ptr> available_kernels);

                ~program();

                detail::raw_context_ptr context_;
                detail::raw_program_ptr program_;
                detail::raw_command_queue_ptr queue_;
                std::map<std::string, detail::raw_kernel_ptr> available_kernels_;
            };

        }    // namespace opencl
    }        // namespace actor
}    // namespace nil
