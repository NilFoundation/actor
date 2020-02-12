#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>

#include <nil/actor/opencl/manager.hpp>
#include <nil/actor/opencl/program.hpp>
#include <nil/actor/opencl/opencl_err.hpp>

namespace nil {
    namespace actor {
        namespace opencl {

            program::program(detail::raw_context_ptr context, detail::raw_command_queue_ptr queue,
                             detail::raw_program_ptr prog,
                             std::map<std::string, detail::raw_kernel_ptr> available_kernels) :
                context_(std::move(context)),
                program_(std::move(prog)), queue_(std::move(queue)), available_kernels_(std::move(available_kernels)) {
                // nop
            }

            program::~program() {
                // nop
            }

        }    // namespace opencl
    }        // namespace actor
}    // namespace nil
