#pragma once

#include <memory>
#include <algorithm>
#include <type_traits>

#include <nil/actor/intrusive_ptr.hpp>

#include <nil/actor/opencl/global.hpp>

#define ACTOR_OPENCL_PTR_ALIAS(aliasname, cltype, claddref, clrelease)                          \
    inline void intrusive_ptr_add_ref(cltype ptr) {                                           \
        claddref(ptr);                                                                        \
    }                                                                                         \
    inline void intrusive_ptr_release(cltype ptr) {                                           \
        clrelease(ptr);                                                                       \
    }                                                                                         \
    namespace nil {                                                                           \
        namespace actor {                                                                       \
            namespace detail {                                                                \
                using aliasname = nil::actor::intrusive_ptr<std::remove_pointer<cltype>::type>; \
            } /* namespace detail */                                                          \
        }     /* namespace actor */                                                             \
    }         // namespace nil

ACTOR_OPENCL_PTR_ALIAS(raw_mem_ptr, cl_mem, clRetainMemObject, clReleaseMemObject)

ACTOR_OPENCL_PTR_ALIAS(raw_event_ptr, cl_event, clRetainEvent, clReleaseEvent)

ACTOR_OPENCL_PTR_ALIAS(raw_kernel_ptr, cl_kernel, clRetainKernel, clReleaseKernel)

ACTOR_OPENCL_PTR_ALIAS(raw_context_ptr, cl_context, clRetainContext, clReleaseContext)

ACTOR_OPENCL_PTR_ALIAS(raw_program_ptr, cl_program, clRetainProgram, clReleaseProgram)

ACTOR_OPENCL_PTR_ALIAS(raw_device_ptr, cl_device_id, clRetainDeviceDummy, clReleaseDeviceDummy)

ACTOR_OPENCL_PTR_ALIAS(raw_command_queue_ptr, cl_command_queue, clRetainCommandQueue, clReleaseCommandQueue)
