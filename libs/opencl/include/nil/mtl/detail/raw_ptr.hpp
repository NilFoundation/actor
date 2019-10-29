#pragma once

#include <memory>
#include <algorithm>
#include <type_traits>

#include <nil/mtl/intrusive_ptr.hpp>

#include <nil/mtl/opencl/global.hpp>

#define MTL_OPENCL_PTR_ALIAS(aliasname, cltype, claddref, clrelease)                          \
    inline void intrusive_ptr_add_ref(cltype ptr) {                                           \
        claddref(ptr);                                                                        \
    }                                                                                         \
    inline void intrusive_ptr_release(cltype ptr) {                                           \
        clrelease(ptr);                                                                       \
    }                                                                                         \
    namespace nil {                                                                           \
        namespace mtl {                                                                       \
            namespace detail {                                                                \
                using aliasname = nil::mtl::intrusive_ptr<std::remove_pointer<cltype>::type>; \
            } /* namespace detail */                                                          \
        }     /* namespace mtl */                                                             \
    }         // namespace nil

MTL_OPENCL_PTR_ALIAS(raw_mem_ptr, cl_mem, clRetainMemObject, clReleaseMemObject)

MTL_OPENCL_PTR_ALIAS(raw_event_ptr, cl_event, clRetainEvent, clReleaseEvent)

MTL_OPENCL_PTR_ALIAS(raw_kernel_ptr, cl_kernel, clRetainKernel, clReleaseKernel)

MTL_OPENCL_PTR_ALIAS(raw_context_ptr, cl_context, clRetainContext, clReleaseContext)

MTL_OPENCL_PTR_ALIAS(raw_program_ptr, cl_program, clRetainProgram, clReleaseProgram)

MTL_OPENCL_PTR_ALIAS(raw_device_ptr, cl_device_id, clRetainDeviceDummy, clReleaseDeviceDummy)

MTL_OPENCL_PTR_ALIAS(raw_command_queue_ptr, cl_command_queue, clRetainCommandQueue, clReleaseCommandQueue)
