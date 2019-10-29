#pragma once

#include <nil/mtl/opencl/global.hpp>

#include <nil/mtl/logger.hpp>

#define MTL_CLF(funname) #funname, funname

namespace nil {
    namespace mtl {
        namespace opencl {

            void throwcl(const char *fname, cl_int err);

            void CL_CALLBACK pfn_notify(const char *errinfo, const void *, size_t, void *);

            // call convention for simply calling a function
            template<class F, class... Ts>
            void v1callcl(const char *fname, F f, Ts &&... vs) {
                throwcl(fname, f(std::forward<Ts>(vs)...));
            }

            // call convention for simply calling a function, not using the last argument
            template<class F, class... Ts>
            void v2callcl(const char *fname, F f, Ts &&... vs) {
                throwcl(fname, f(std::forward<Ts>(vs)..., nullptr));
            }

            // call convention for simply calling a function, and logging errors
            template<class F, class... Ts>
            void v3callcl(F f, Ts &&... vs) {
                auto err = f(std::forward<Ts>(vs)...);
                if (err != CL_SUCCESS)
                    MTL_LOG_ERROR("error: " << opencl_error(err));
            }

            // call convention with `result` argument at the end returning `err`, not
            // using the second last argument (set to nullptr) nor the one before (set to 0)
            template<class R, class F, class... Ts>
            R v1get(const char *fname, F f, Ts &&... vs) {
                R result;
                throwcl(fname, f(std::forward<Ts>(vs)..., cl_uint {0}, nullptr, &result));
                return result;
            }

            // call convention with `err` argument at the end returning `result`
            template<class F, class... Ts>
            auto v2get(const char *fname, F f, Ts &&... vs) -> decltype(f(std::forward<Ts>(vs)..., nullptr)) {
                cl_int err;
                auto result = f(std::forward<Ts>(vs)..., &err);
                throwcl(fname, err);
                return result;
            }

            // call convention with `result` argument at second last position (preceeded by
            // its size) followed by an ingored void* argument (nullptr) returning `err`
            template<class R, class F, class... Ts>
            R v3get(const char *fname, F f, Ts &&... vs) {
                R result;
                throwcl(fname, f(std::forward<Ts>(vs)..., sizeof(R), &result, nullptr));
                return result;
            }

        }    // namespace opencl
    }        // namespace mtl
}    // namespace nil
