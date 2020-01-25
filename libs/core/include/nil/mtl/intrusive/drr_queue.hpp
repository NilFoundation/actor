//---------------------------------------------------------------------------//
// Copyright (c) 2011-2017 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#pragma once

#include <utility>

#include <nil/mtl/intrusive/new_round_result.hpp>
#include <nil/mtl/intrusive/task_queue.hpp>
#include <nil/mtl/intrusive/task_result.hpp>

namespace nil {
    namespace mtl {
        namespace intrusive {

            /// A Deficit Round Robin queue.
            template<class Policy>
            class drr_queue : public task_queue<Policy> {
            public:
                // -- member types -----------------------------------------------------------

                using super = task_queue<Policy>;

                using typename super::policy_type;

                using typename super::unique_pointer;

                using typename super::value_type;

                using deficit_type = typename policy_type::deficit_type;

                // -- constructors, destructors, and assignment operators --------------------

                drr_queue(policy_type p) : super(std::move(p)), deficit_(0) {
                    // nop
                }

                drr_queue(drr_queue &&other) : super(std::move(other)), deficit_(0) {
                    // nop
                }

                drr_queue &operator=(drr_queue &&other) {
                    super::operator=(std::move(other));
                    return *this;
                }

                // -- observers --------------------------------------------------------------

                deficit_type deficit() const {
                    return deficit_;
                }

                // -- modifiers --------------------------------------------------------------

                void inc_deficit(deficit_type x) noexcept {
                    if (!super::empty())
                        deficit_ += x;
                }

                void flush_cache() const noexcept {
                    // nop
                }

                /// Consumes items from the queue until the queue is empty or there is not
                /// enough deficit to dequeue the next task.
                /// @returns `true` if `f` consumed at least one item.
                template<class F>
                bool consume(F &f) noexcept(noexcept(f(std::declval<value_type &>()))) {
                    auto res = new_round(0, f);
                    return res.consumed_items;
                }

                /// Takes the first element out of the queue if the deficit allows it and
                /// returns the element.
                /// @private
                unique_pointer next() noexcept {
                    return super::next(deficit_);
                }

                /// Run a new round with `quantum`, dispatching all tasks to `consumer`.
                /// @returns `true` if at least one item was consumed, `false` otherwise.
                template<class F>
                new_round_result new_round(deficit_type quantum, F &consumer) {
                    if (!super::empty()) {
                        deficit_ += quantum;
                        auto ptr = next();
                        if (ptr == nullptr)
                            return {false, false};
                        do {
                            switch (consumer(*ptr)) {
                                default:
                                    break;
                                case task_result::stop:
                                    return {true, false};
                                case task_result::stop_all:
                                    return {true, true};
                            }
                            ptr = next();
                        } while (ptr != nullptr);
                        return {true, false};
                    }
                    return {false, false};
                }

            private:
                // -- member variables -------------------------------------------------------

                /// Stores the deficit on this queue.
                deficit_type deficit_ = 0;
            };
        }    // namespace intrusive
    }        // namespace mtl
}    // namespace nil
