//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt for Boost License or
// http://opensource.org/licenses/BSD-3-Clause for BSD 3-Clause License
//---------------------------------------------------------------------------//

#pragma once

#include <cstdint>

#include <nil/actor/detail/comparable.hpp>
#include <nil/actor/fwd.hpp>
#include <nil/actor/ipv4_address.hpp>
#include <nil/actor/ipv4_subnet.hpp>
#include <nil/actor/ipv6_address.hpp>

namespace nil {
    namespace actor {

        class ipv6_subnet : detail::comparable<ipv6_subnet> {
        public:
            // -- constants --------------------------------------------------------------

            /// Stores the offset of an embedded IPv4 subnet in bits.
            static constexpr uint8_t v4_offset =
                static_cast<uint8_t>(ipv6_address::num_bytes - ipv4_address::num_bytes) * 8;

            // -- constructors, destructors, and assignment operators --------------------

            ipv6_subnet();

            ipv6_subnet(const ipv6_subnet &) = default;

            explicit ipv6_subnet(ipv4_subnet subnet);

            ipv6_subnet(ipv4_address network_address, uint8_t prefix_length);

            ipv6_subnet(ipv6_address network_address, uint8_t prefix_length);

            ipv6_subnet &operator=(const ipv6_subnet &) = default;

            // -- properties -------------------------------------------------------------

            /// Returns whether this subnet embeds an IPv4 subnet.
            bool embeds_v4() const noexcept;

            /// Returns an embedded IPv4 subnet.
            /// @pre `embeds_v4()`
            ipv4_subnet embedded_v4() const noexcept;

            /// Returns the network address for this subnet.
            inline const ipv6_address &network_address() const noexcept {
                return address_;
            }

            /// Returns the prefix length of the netmask.
            inline uint8_t prefix_length() const noexcept {
                return prefix_length_;
            }

            /// Returns whether `addr` belongs to this subnet.
            bool contains(ipv6_address addr) const noexcept;

            /// Returns whether this subnet includes `other`.
            bool contains(ipv6_subnet other) const noexcept;

            /// Returns whether `addr` belongs to this subnet.
            bool contains(ipv4_address addr) const noexcept;

            /// Returns whether this subnet includes `other`.
            bool contains(ipv4_subnet other) const noexcept;

            // -- comparison -------------------------------------------------------------

            int compare(const ipv6_subnet &other) const noexcept;

            // -- inspection -------------------------------------------------------------

            template<class Inspector>
            friend typename Inspector::result_type inspect(Inspector &f, ipv6_subnet &x) {
                return f(x.address_, x.prefix_length_);
            }

        private:
            // -- member variables -------------------------------------------------------

            ipv6_address address_;
            uint8_t prefix_length_;
        };

        // -- related free functions ---------------------------------------------------

        /// @relates ipv6_subnet
        std::string to_string(ipv6_subnet x);

    }    // namespace actor
}    // namespace nil
