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

#include <new>
#include <atomic>
#include <cstddef>
#include <type_traits>

#include <nil/mtl/config.hpp>
#include <nil/mtl/abstract_actor.hpp>
#include <nil/mtl/actor_control_block.hpp>

#ifdef MTL_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

namespace nil {
    namespace mtl {

        template<class T>
        class actor_storage {
        public:
            template<class... Us>
            actor_storage(actor_id x, node_id y, actor_system *sys, Us &&... zs) :
                ctrl(x, y, sys, data_dtor, block_dtor) {
                // construct data member
                new (&data) T(std::forward<Us>(zs)...);
            }

            ~actor_storage() {
                // 1) make sure control block fits into a single cache line
                static_assert(sizeof(actor_control_block) < MTL_CACHE_LINE_SIZE,
                              "actor_control_block exceeds a single cache line");
// Clang in combination with libc++ on Linux complains about offsetof:
//     error: 'actor_storage' does not refer to a value
// Until we have found a reliable solution, we disable this safety check.
#if !(defined(MTL_CLANG) && defined(MTL_LINUX))
                // 2) make sure reinterpret cast of the control block to the storage works
                static_assert(offsetof(actor_storage, ctrl) == 0, "control block is not at the start of the storage");
                // 3) make sure we can obtain a data pointer by jumping one cache line
                static_assert(offsetof(actor_storage, data) == MTL_CACHE_LINE_SIZE,
                              "data is not at cache line size boundary");
#else
                // 4) make sure static_cast and reinterpret_cast
                //    between T* and abstract_actor* are identical
                constexpr abstract_actor *dummy = nullptr;
                constexpr T *derived_dummy = static_cast<T *>(dummy);
                static_assert(derived_dummy == nullptr,
                              "actor subtype has illegal memory alignment "
                              "(probably due to virtual inheritance)");
#endif
            }

            actor_storage(const actor_storage &) = delete;
            actor_storage &operator=(const actor_storage &) = delete;

            static_assert(sizeof(actor_control_block) < MTL_CACHE_LINE_SIZE, "actor_control_block exceeds 64 bytes");

            actor_control_block ctrl;
            char pad[MTL_CACHE_LINE_SIZE - sizeof(actor_control_block)];
            union {
                T data;
            };

        private:
            static void data_dtor(abstract_actor *ptr) {
                // safe due to static assert #3
                ptr->on_destroy();
                static_cast<T *>(ptr)->~T();
            }

            static void block_dtor(actor_control_block *ptr) {
                // safe due to static assert #2
                delete reinterpret_cast<actor_storage *>(ptr);
            }
        };

        /// @relates actor_storage
        template<class T>
        bool intrusive_ptr_upgrade_weak(actor_storage<T> *x) {
            auto count = x->ctrl.strong_refs.load();
            while (count != 0) {
                if (x->ctrl.strong_refs.compare_exchange_weak(count, count + 1, std::memory_order_relaxed))
                    return true;
            }
            return false;
        }

        /// @relates actor_storage
        template<class T>
        void intrusive_ptr_add_weak_ref(actor_storage<T> *x) {
            x->ctrl.weak_refs.fetch_add(1, std::memory_order_relaxed);
        }

        /// @relates actor_storage
        template<class T>
        void intrusive_ptr_release_weak(actor_storage<T> *x) {
            // destroy object if last weak pointer expires
            if (x->ctrl.weak_refs == 1 || x->ctrl.weak_refs.fetch_sub(1, std::memory_order_acq_rel) == 1)
                delete x;
        }

        /// @relates actor_storage
        template<class T>
        void intrusive_ptr_add_ref(actor_storage<T> *x) {
            x->ctrl.strong_refs.fetch_add(1, std::memory_order_relaxed);
        }

        /// @relates actor_storage
        template<class T>
        void intrusive_ptr_release(actor_storage<T> *x) {
            // release implicit weak pointer if the last strong ref expires
            // and destroy the data block
            if (x->ctrl.strong_refs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                x->destroy_data();
                intrusive_ptr_relase_weak(x);
            }
        }

    }    // namespace mtl
}    // namespace nil

#ifdef MTL_GCC
#pragma GCC diagnostic pop
#endif
