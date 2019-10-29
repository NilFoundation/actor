#include <utility>
#include <vector>
#include <iostream>

#include <nil/mtl/opencl/platform.hpp>
#include <nil/mtl/opencl/opencl_err.hpp>

namespace nil {
    namespace mtl {
        namespace opencl {

            platform_ptr platform::create(cl_platform_id platform_id, unsigned start_id) {
                std::vector<unsigned> device_types = {CL_DEVICE_TYPE_GPU, CL_DEVICE_TYPE_ACCELERATOR,
                                                      CL_DEVICE_TYPE_CPU};
                std::vector<cl_device_id> ids;
                for (cl_device_type device_type : device_types) {
                    auto known = ids.size();
                    cl_uint discoverd;
                    auto err = clGetDeviceIDs(platform_id, device_type, 0, nullptr, &discoverd);
                    if (err == CL_DEVICE_NOT_FOUND) {
                        continue;    // no devices of the type found
                    } else if (err != CL_SUCCESS) {
                        throwcl("clGetDeviceIDs", err);
                    }
                    ids.resize(known + discoverd);
                    v2callcl(MTL_CLF(clGetDeviceIDs), platform_id, device_type, discoverd, (ids.data() + known));
                }
                std::vector<detail::raw_device_ptr> devices;
                devices.resize(ids.size());
                auto lift = [](cl_device_id ptr) { return detail::raw_device_ptr {ptr, false}; };
                std::transform(ids.begin(), ids.end(), devices.begin(), lift);
                detail::raw_context_ptr context;
                context.reset(v2get(MTL_CLF(clCreateContext), nullptr, static_cast<unsigned>(ids.size()), ids.data(),
                                    pfn_notify, nullptr),
                              false);
                std::vector<device_ptr> device_information;
                for (auto &device_id : devices) {
                    device_information.push_back(device::create(context, device_id, start_id++));
                }
                if (device_information.empty())
                    MTL_RAISE_ERROR("no devices for the platform found");
                auto name = platform_info(platform_id, CL_PLATFORM_NAME);
                auto vendor = platform_info(platform_id, CL_PLATFORM_VENDOR);
                auto version = platform_info(platform_id, CL_PLATFORM_VERSION);
                return make_counted<platform>(platform_id, std::move(context), move(name), move(vendor), move(version),
                                              std::move(device_information));
            }

            std::string platform::platform_info(cl_platform_id platform_id, unsigned info_flag) {
                size_t size;
                auto err = clGetPlatformInfo(platform_id, info_flag, 0, nullptr, &size);
                throwcl("clGetPlatformInfo", err);
                std::vector<char> buffer(size);
                v2callcl(MTL_CLF(clGetPlatformInfo), platform_id, info_flag, sizeof(char) * size, buffer.data());
                return std::string(buffer.data());
            }

            platform::platform(cl_platform_id platform_id, detail::raw_context_ptr context, std::string name,
                               std::string vendor, std::string version, std::vector<device_ptr> devices) :
                platform_id_(platform_id),
                context_(std::move(context)), name_(move(name)), vendor_(move(vendor)), version_(move(version)),
                devices_(std::move(devices)) {
                // nop
            }

            platform::~platform() {
                // nop
            }

        }    // namespace opencl
    }        // namespace mtl
}    // namespace nil
