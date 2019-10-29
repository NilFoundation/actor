#pragma once

#include <nil/mtl/ref_counted.hpp>

#include <nil/mtl/opencl/device.hpp>

namespace nil {
    namespace mtl {
        namespace opencl {

            class platform;

            using platform_ptr = intrusive_ptr<platform>;

            class platform : public ref_counted {
            public:
                friend class program;

                template<class T, class... Ts>
                friend intrusive_ptr<T> nil::mtl::make_counted(Ts &&...);

                inline const std::vector<device_ptr> &devices() const;

                inline const std::string &name() const;

                inline const std::string &vendor() const;

                inline const std::string &version() const;

                static platform_ptr create(cl_platform_id platform_id, unsigned start_id);

            private:
                platform(cl_platform_id platform_id, detail::raw_context_ptr context, std::string name,
                         std::string vendor, std::string version, std::vector<device_ptr> devices);

                ~platform();

                static std::string platform_info(cl_platform_id platform_id, unsigned info_flag);

                cl_platform_id platform_id_;
                detail::raw_context_ptr context_;
                std::string name_;
                std::string vendor_;
                std::string version_;
                std::vector<device_ptr> devices_;
            };

            /******************************************************************************\
             *                 implementation of inline member functions                  *
            \******************************************************************************/

            inline const std::vector<device_ptr> &platform::devices() const {
                return devices_;
            }

            inline const std::string &platform::name() const {
                return name_;
            }

            inline const std::string &platform::vendor() const {
                return vendor_;
            }

            inline const std::string &platform::version() const {
                return version_;
            }

        }    // namespace opencl
    }        // namespace mtl
}    // namespace nil
