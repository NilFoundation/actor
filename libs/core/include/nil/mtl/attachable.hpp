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

#include <memory>
#include <cstdint>
#include <typeinfo>

#include <nil/mtl/error.hpp>
#include <nil/mtl/optional.hpp>
#include <nil/mtl/exit_reason.hpp>
#include <nil/mtl/execution_unit.hpp>

namespace nil {
    namespace mtl {

        class abstract_actor;

        /// Callback utility class.
        class attachable {
        public:
            attachable() = default;
            attachable(const attachable &) = delete;
            attachable &operator=(const attachable &) = delete;

            /// Represents a pointer to a value with its subtype as type ID number.
            struct token {
                /// Identifies a non-matchable subtype.
                static constexpr size_t anonymous = 0;

                /// Identifies `abstract_group::subscription`.
                static constexpr size_t subscription = 1;

                /// Identifies `default_attachable::observe_token`.
                static constexpr size_t observer = 2;

                /// Identifies `stream_aborter::token`.
                static constexpr size_t stream_aborter = 3;

                template<class T>
                token(const T &tk) : subtype(T::token_type), ptr(&tk) {
                    // nop
                }

                /// Denotes the type of ptr.
                size_t subtype;

                /// Any value, used to identify attachable instances.
                const void *ptr;

                token(size_t typenr, const void *vptr);
            };

            virtual ~attachable();

            /// Executed if the actor finished execution with given `reason`.
            /// The default implementation does nothing.
            /// @warning `host` can be `nullptr`
            virtual void actor_exited(const error &fail_state, execution_unit *host);

            /// Returns `true` if `what` selects this instance, otherwise `false`.
            virtual bool matches(const token &what);

            /// Returns `true` if `what` selects this instance, otherwise `false`.
            template<class T>
            bool matches(const T &what) {
                return matches(token {T::token_type, &what});
            }

            std::unique_ptr<attachable> next;
        };

        /// @relates attachable
        using attachable_ptr = std::unique_ptr<attachable>;

    }    // namespace mtl
}    // namespace nil
