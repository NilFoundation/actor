#include <nil/actor/opencl/opencl_err.hpp>

#include <nil/actor/logger.hpp>

namespace nil {
    namespace actor {
        namespace opencl {

            void throwcl(const char *, cl_int err) {
                if (err != CL_SUCCESS) {
                    ACTOR_RAISE_ERROR("throwcl: unrecoverable OpenCL error");
                }
            }

            void CL_CALLBACK pfn_notify(const char *errinfo, const void *, size_t, void *) {
                ACTOR_LOG_ERROR("\n##### Error message via pfn_notify #####\n"
                              << errinfo << "\n########################################");
                ACTOR_IGNORE_UNUSED(errinfo);
            }

        }    // namespace opencl
    }        // namespace actor
}    // namespace nil
