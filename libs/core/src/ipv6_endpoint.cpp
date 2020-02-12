//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2019 Nil Foundation AG
// Copyright (c) 2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.
//---------------------------------------------------------------------------//

#include <nil/actor/ipv6_endpoint.hpp>

#include <nil/actor/detail/fnv_hash.hpp>
#include <nil/actor/ipv4_address.hpp>
#include <nil/actor/ipv4_endpoint.hpp>

namespace nil {
    namespace actor {

        ipv6_endpoint::ipv6_endpoint(ipv6_address address, uint16_t port) : address_(address), port_(port) {
            // nop
        }

        ipv6_endpoint::ipv6_endpoint(ipv4_address address, uint16_t port) : address_(address), port_(port) {
            // nop
        }

        size_t ipv6_endpoint::hash_code() const noexcept {
            auto result = detail::fnv_hash(address_.data());
            return detail::fnv_hash_append(result, port_);
        }

        long ipv6_endpoint::compare(ipv6_endpoint x) const noexcept {
            auto res = address_.compare(x.address());
            return res == 0 ? port_ - x.port() : res;
        }

        long ipv6_endpoint::compare(ipv4_endpoint x) const noexcept {
            ipv6_endpoint y {x.address(), x.port()};
            return compare(y);
        }

        std::string to_string(const ipv6_endpoint &x) {
            std::string result;
            auto addr = x.address();
            if (addr.embeds_v4()) {
                result += to_string(addr);
                result += ":";
                result += std::to_string(x.port());
            } else {
                result += '[';
                result += to_string(addr);
                result += "]:";
                result += std::to_string(x.port());
            }
            return result;
        }

    }    // namespace actor
}    // namespace nil
