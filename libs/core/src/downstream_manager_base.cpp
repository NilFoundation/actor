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

#include <nil/actor/downstream_manager_base.hpp>

#include <functional>

#include <nil/actor/logger.hpp>
#include <nil/actor/outbound_path.hpp>

namespace nil {
    namespace actor {

        downstream_manager_base::downstream_manager_base(stream_manager *parent) : super(parent) {
            // nop
        }

        downstream_manager_base::~downstream_manager_base() {
            // nop
        }

        size_t downstream_manager_base::num_paths() const noexcept {
            return paths_.size();
        }

        bool downstream_manager_base::remove_path(stream_slot slot, error reason, bool silent) noexcept {
            ACTOR_LOG_TRACE(ACTOR_ARG(slot) << ACTOR_ARG(reason) << ACTOR_ARG(silent));
            auto i = paths_.find(slot);
            if (i != paths_.end()) {
                about_to_erase(i->second.get(), silent, reason ? &reason : nullptr);
                paths_.erase(i);
                return true;
            }
            return false;
        }

        auto downstream_manager_base::path(stream_slot slot) noexcept -> path_ptr {
            auto i = paths_.find(slot);
            return i != paths_.end() ? i->second.get() : nullptr;
        }

        void downstream_manager_base::clear_paths() {
            paths_.clear();
        }

        bool downstream_manager_base::insert_path(unique_path_ptr ptr) {
            ACTOR_LOG_TRACE(ACTOR_ARG(ptr));
            ACTOR_ASSERT(ptr != nullptr);
            auto slot = ptr->slots.sender;
            ACTOR_ASSERT(slot != invalid_stream_slot);
            return paths_.emplace(slot, std::move(ptr)).second;
        }

        void downstream_manager_base::for_each_path_impl(path_visitor &f) {
            for (auto &kvp : paths_)
                f(*kvp.second);
        }

        bool downstream_manager_base::check_paths_impl(path_algorithm algo, path_predicate &pred) const noexcept {
            auto f = [&](const map_type::value_type &x) { return pred(*x.second); };
            switch (algo) {
                default:    // all_of
                    return std::all_of(paths_.begin(), paths_.end(), f);
                case path_algorithm::any_of:
                    return std::any_of(paths_.begin(), paths_.end(), f);
                case path_algorithm::none_of:
                    return std::none_of(paths_.begin(), paths_.end(), f);
            }
        }

    }    // namespace actor
}    // namespace nil
