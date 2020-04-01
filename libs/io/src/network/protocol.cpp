//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/io/network/protocol.hpp>

namespace nil {
    namespace actor {
        namespace io {
            namespace network {

                std::string to_string(const protocol &x) {
                    std::string result;
                    result += to_string(x.trans);
                    result += "/";
                    result += to_string(x.net);
                    return result;
                }

            }    // namespace network
        }        // namespace io
    }            // namespace actor
}    // namespace nil
