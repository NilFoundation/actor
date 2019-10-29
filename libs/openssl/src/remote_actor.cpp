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

#include <nil/mtl/openssl/remote_actor.hpp>

#include <nil/mtl/sec.hpp>
#include <nil/mtl/atom.hpp>
#include <nil/mtl/expected.hpp>
#include <nil/mtl/function_view.hpp>

#include <nil/mtl/openssl/manager.hpp>

namespace nil {
    namespace mtl {
        namespace openssl {

            /// Establish a new connection to the actor at `host` on given `port`.
            /// @param host Valid hostname or IP address.
            /// @param port TCP port.
            /// @returns An `actor` to the proxy instance representing
            ///          a remote actor or an `error`.
            expected<strong_actor_ptr> remote_actor(actor_system &sys, const std::set<std::string> &mpi,
                                                    std::string host, uint16_t port) {
                MTL_LOG_TRACE(MTL_ARG(mpi) << MTL_ARG(host) << MTL_ARG(port));
                expected<strong_actor_ptr> res {strong_actor_ptr {nullptr}};
                auto f = make_function_view(sys.openssl_manager().actor_handle());
                auto x = f(connect_atom::value, std::move(host), port);
                if (!x)
                    return std::move(x.error());
                auto &tup = *x;
                auto &ptr = get<1>(tup);
                if (!ptr)
                    return sec::no_actor_published_at_port;
                auto &found_mpi = get<2>(tup);
                if (sys.assignable(found_mpi, mpi))
                    return std::move(ptr);
                return sec::unexpected_actor_messaging_interface;
            }

        }    // namespace openssl
    }        // namespace mtl
}
