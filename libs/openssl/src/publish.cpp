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

#include <nil/mtl/openssl/publish.hpp>

#include <set>

#include <nil/mtl/atom.hpp>
#include <nil/mtl/expected.hpp>
#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/function_view.hpp>
#include <nil/mtl/actor_control_block.hpp>

#include <nil/mtl/openssl/manager.hpp>

namespace nil {
    namespace mtl {
        namespace openssl {

            expected<uint16_t> publish(actor_system &sys, const strong_actor_ptr &whom, std::set<std::string> &&sigs,
                                       uint16_t port, const char *cstr, bool ru) {
                MTL_LOG_TRACE(MTL_ARG(whom) << MTL_ARG(sigs) << MTL_ARG(port));
                MTL_ASSERT(whom != nullptr);
                std::string in;
                if (cstr != nullptr)
                    in = cstr;
                auto f = make_function_view(sys.openssl_manager().actor_handle());
                return f(publish_atom::value, port, std::move(whom), std::move(sigs), std::move(in), ru);
            }

        }    // namespace openssl
    }        // namespace mtl
}