#include <nil/mtl/network/operation.hpp>

#include <string>

namespace nil {
    namespace mtl {
        namespace net {

            std::string to_string(operation x) {
                switch (x) {
                    default:
                        return "???";
                    case operation::none:
                        return "none";
                    case operation::read:
                        return "read";
                    case operation::write:
                        return "write";
                    case operation::read_write:
                        return "read_write";
                    case operation::shutdown:
                        return "shutdown";
                };
            }

        }    // namespace net
    }        // namespace mtl
}