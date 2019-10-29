#include <nil/mtl/opencl/opencl_err.hpp>

#include <nil/mtl/logger.hpp>

namespace nil {
    namespace mtl {
        namespace opencl {

            void throwcl(const char *, cl_int err) {
                if (err != CL_SUCCESS) {
                    MTL_RAISE_ERROR("throwcl: unrecoverable OpenCL error");
                }
            }

            void CL_CALLBACK pfn_notify(const char *errinfo, const void *, size_t, void *) {
                MTL_LOG_ERROR("\n##### Error message via pfn_notify #####\n"
                              << errinfo << "\n########################################");
                MTL_IGNORE_UNUSED(errinfo);
            }

        }    // namespace opencl
    }        // namespace mtl
}    // namespace nil
