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

#include <tuple>
#include <sstream>
#include <type_traits>

#include <nil/mtl/atom.hpp>
#include <nil/mtl/config.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/index_mapping.hpp>
#include <nil/mtl/make_counted.hpp>
#include <nil/mtl/none.hpp>
#include <nil/mtl/optional.hpp>
#include <nil/mtl/skip.hpp>
#include <nil/mtl/type_nr.hpp>

#include <nil/mtl/detail/apply_args.hpp>
#include <nil/mtl/detail/comparable.hpp>
#include <nil/mtl/detail/implicit_conversions.hpp>
#include <nil/mtl/detail/int_list.hpp>
#include <nil/mtl/detail/message_data.hpp>
#include <nil/mtl/detail/type_traits.hpp>

namespace nil {
    namespace mtl {
        class message_handler;

        /// Describes a fixed-length, copy-on-write, type-erased
        /// tuple with elements of any type.
        class message : public type_erased_tuple {
        public:
            // -- member types -----------------------------------------------------------

            /// Raw pointer to content.
            using raw_ptr = detail::message_data *;

            /// Copy-on-write pointer to content.
            using data_ptr = detail::message_data::cow_ptr;

            // -- constructors, destructors, and assignment operators --------------------

            message() noexcept = default;
            message(none_t) noexcept;
            message(const message &) = default;
            message &operator=(const message &) = default;

            message(message &&) noexcept;
            message &operator=(message &&) noexcept;
            explicit message(data_ptr ptr) noexcept;

            ~message() override;

            // -- implementation of type_erased_tuple ------------------------------------

            void *get_mutable(size_t p) override;

            error load(size_t pos, deserializer &source) override;

            size_t size() const noexcept override;

            uint32_t type_token() const noexcept override;

            rtti_pair type(size_t pos) const noexcept override;

            const void *get(size_t pos) const noexcept override;

            std::string stringify(size_t pos) const override;

            type_erased_value_ptr copy(size_t pos) const override;

            error save(size_t pos, serializer &sink) const override;

            bool shared() const noexcept override;

            error load(deserializer &source) override;

            error save(serializer &sink) const override;

            // -- factories --------------------------------------------------------------

            /// Creates a new message by concatenating `xs...`.
            template<class... Ts>
            static message concat(const Ts &... xs) {
                return concat_impl({xs.vals()...});
            }

            /// Creates a new message by copying all elements in a type-erased tuple.
            static message copy(const type_erased_tuple &xs);

            // -- modifiers --------------------------------------------------------------

            /// Concatenates `*this` and `x`.
            message &operator+=(const message &x);

            /// Returns `handler(*this)`.
            optional<message> apply(message_handler handler);

            /// Forces the message to copy its content if there are more than
            /// one references to the content.
            inline void force_unshare() {
                vals_.unshare();
            }

            /// Returns a mutable reference to the content. Callers are responsible
            /// for unsharing content if necessary.
            inline data_ptr &vals() {
                return vals_;
            }

            /// Exchanges content of `this` and `other`.
            void swap(message &other) noexcept;

            /// Assigns new content.
            void reset(raw_ptr new_ptr = nullptr, bool add_ref = true) noexcept;

            // -- observers --------------------------------------------------------------

            /// Creates a new message with all but the first n values.
            message drop(size_t n) const;

            /// Creates a new message with all but the last n values.
            message drop_right(size_t n) const;

            /// Creates a new message of size `n` starting at the element at position `p`.
            message slice(size_t pos, size_t n) const;

            /// Filters this message by applying slices of it to `handler` and  returns
            /// the remaining elements of this operation. Slices are generated in the
            /// sequence `[0, size)`, `[0, size-1)`, `...` , `[1, size-1)`, `...`,
            /// `[size-1, size)`. Whenever a slice matches, it is removed from the message
            /// and the next slice starts at the *same* index on the reduced message.
            ///
            /// For example:
            ///
            /// ~~~
            /// auto msg = make_message(1, 2.f, 3.f, 4);
            /// // extract float and integer pairs
            /// auto msg2 = msg.extract({
            ///   [](float, float) { },
            ///   [](int, int) { }
            /// });
            /// assert(msg2 == make_message(1, 4));
            /// ~~~
            ///
            /// Step-by-step explanation:
            /// - Slice 1: `(1, 2.f, 3.f, 4)`, no match
            /// - Slice 2: `(1, 2.f, 3.f)`, no match
            /// - Slice 3: `(1, 2.f)`, no match
            /// - Slice 4: `(1)`, no match
            /// - Slice 5: `(2.f, 3.f, 4)`, no match
            /// - Slice 6: `(2.f, 3.f)`, *match*; new message is `(1, 4)`
            /// - Slice 7: `(4)`, no match
            ///
            /// Slice 7 is `(4)`, i.e., does not contain the first element, because the
            /// match on slice 6 occurred at index position 1. The function `extract`
            /// iterates a message only once, from left to right.
            message extract(message_handler handler) const;

            // -- inline observers -------------------------------------------------------

            /// Returns a const pointer to the element at position `p`.
            inline const void *at(size_t p) const noexcept {
                MTL_ASSERT(vals_ != nullptr);
                return vals_->get(p);
            }

            /// Returns a reference to the content.
            inline const data_ptr &vals() const noexcept {
                return vals_;
            }

            /// Returns a reference to the content.
            inline const data_ptr &cvals() const noexcept {
                return vals_;
            }

            /// Returns the size of this message.
            /// Creates a new message from the first n values.
            inline message take(size_t n) const {
                return n >= size() ? *this : drop_right(size() - n);
            }

            /// Creates a new message from the last n values.
            inline message take_right(size_t n) const {
                return n >= size() ? *this : drop(size() - n);
            }

            /// @cond PRIVATE

            /// @pre `!empty()`
            inline type_erased_tuple &content() {
                MTL_ASSERT(vals_ != nullptr);
                return vals_.unshared();
            }

            inline const type_erased_tuple &content() const {
                MTL_ASSERT(vals_ != nullptr);
                return *vals_;
            }

            /// Serializes the content of `x` as if `x` was an instance of `message`. The
            /// resulting output of `sink` can then be used to deserialize a `message`
            /// even if the serialized object had a different type.
            static error save(serializer &sink, const type_erased_tuple &x);

            /// @endcond

        private:
            // -- private helpers --------------------------------------------------------

            template<size_t P>
            static bool match_elements_impl(std::integral_constant<size_t, P>, detail::type_list<>) noexcept {
                return true;    // end of recursion
            }

            template<size_t P, class T, class... Ts>
            bool match_elements_impl(std::integral_constant<size_t, P>, detail::type_list<T, Ts...>) const noexcept {
                std::integral_constant<size_t, P + 1> next_p;
                detail::type_list<Ts...> next_list;
                return match_element<T>(P) && match_elements_impl(next_p, next_list);
            }

            message extract_impl(size_t start, message_handler handler) const;

            static message concat_impl(std::initializer_list<data_ptr> xs);

            // -- member functions -------------------------------------------------------

            data_ptr vals_;
        };

        // -- related non-members ------------------------------------------------------

        /// @relates message
        error inspect(serializer &sink, message &msg);

        /// @relates message
        error inspect(deserializer &source, message &msg);

        /// @relates message
        std::string to_string(const message &msg);

        /// @relates message
        inline message operator+(const message &lhs, const message &rhs) {
            return message::concat(lhs, rhs);
        }

    }    // namespace mtl
}    // namespace nil
