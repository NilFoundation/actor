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

#include <type_traits>

#include <nil/mtl/fwd.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/ref_counted.hpp>
#include <nil/mtl/infer_handle.hpp>
#include <nil/mtl/intrusive_ptr.hpp>
#include <nil/mtl/actor_storage.hpp>
#include <nil/mtl/actor_control_block.hpp>

namespace nil {
    namespace mtl {

        template<class T, class R = infer_handle_from_class_t<T>, class... Ts>
        R make_actor(actor_id aid, node_id nid, spawner *sys, Ts &&... xs) {
#if MTL_LOG_LEVEL >= MTL_LOG_LEVEL_DEBUG
            actor_storage<T> *ptr = nullptr;
            if (logger::current_logger()->accepts(MTL_LOG_LEVEL_DEBUG, nil::mtl::atom(MTL_LOG_FLOW_COMPONENT))) {
                std::string args;
                args = deep_to_string(std::forward_as_tuple(xs...));
                ptr = new actor_storage<T>(aid, std::move(nid), sys, std::forward<Ts>(xs)...);
                MTL_LOG_SPAWN_EVENT(ptr->data, args);
            } else {
                ptr = new actor_storage<T>(aid, std::move(nid), sys, std::forward<Ts>(xs)...);
            }
#else
            auto ptr = new actor_storage<T>(aid, std::move(nid), sys, std::forward<Ts>(xs)...);
#endif
            return {&(ptr->ctrl), false};
        }

    }    // namespace mtl
}    // namespace nil
