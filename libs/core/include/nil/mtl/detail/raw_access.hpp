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

#include <nil/mtl/actor.hpp>
#include <nil/mtl/group.hpp>
#include <nil/mtl/channel.hpp>
#include <nil/mtl/actor_addr.hpp>
#include <nil/mtl/abstract_actor.hpp>
#include <nil/mtl/abstract_group.hpp>
#include <nil/mtl/abstract_channel.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            class raw_access {

            public:
                template<class ActorHandle>
                static abstract_actor *get(const ActorHandle &hdl) {
                    return hdl.ptr_.get();
                }

                static abstract_channel *get(const channel &hdl) {
                    return hdl.ptr_.get();
                }

                static abstract_group *get(const group &hdl) {
                    return hdl.ptr_.get();
                }

                static actor unsafe_cast(abstract_actor *ptr) {
                    return {ptr};
                }

                static actor unsafe_cast(const actor_addr &hdl) {
                    return {get(hdl)};
                }

                static actor unsafe_cast(const abstract_actor_ptr &ptr) {
                    return {ptr.get()};
                }

                template<class T>
                static void unsafe_assign(T &lhs, const actor &rhs) {
                    lhs = T {get(rhs)};
                }

                template<class T>
                static void unsafe_assign(T &lhs, const abstract_actor_ptr &ptr) {
                    lhs = T {ptr.get()};
                }
            };

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
