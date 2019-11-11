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

#include <vector>

#include <nil/mtl/fwd.hpp>
#include <nil/mtl/make_message.hpp>
#include <nil/mtl/message.hpp>
#include <nil/mtl/type_erased_value.hpp>
#include <nil/mtl/intrusive_cow_ptr.hpp>

namespace nil {
    namespace mtl {

        /// Provides a convenient interface for creating `message` objects
        /// from a series of values using the member function `append`.
        class message_builder {
        public:
            friend class message;

            message_builder(const message_builder &) = delete;
            message_builder &operator=(const message_builder &) = delete;

            message_builder();
            ~message_builder();

            /// Creates a new instance and immediately calls `append(first, last)`.
            template<class Iter>
            message_builder(Iter first, Iter last) {
                init();
                append(first, last);
            }

            /// Appends all values in range [first, last).
            template<class Iter>
            message_builder &append(Iter first, Iter last) {
                for (; first != last; ++first)
                    append(*first);
                return *this;
            }

            /// Adds `x` to the elements of the buffer.
            template<class T>
            message_builder &append(T &&x) {
                using type = typename unbox_message_element<
                    typename detail::implicit_conversions<typename std::decay<T>::type>::type>::type;
                return emplace(make_type_erased_value<type>(std::forward<T>(x)));
            }

            inline message_builder &append_all() {
                return *this;
            }

            template<class T, class... Ts>
            message_builder &append_all(T &&x, Ts &&... xs) {
                append(std::forward<T>(x));
                return append_all(std::forward<Ts>(xs)...);
            }

            template<size_t N, class... Ts>
            message_builder &append_tuple(std::integral_constant<size_t, N>, std::integral_constant<size_t, N>,
                                          std::tuple<Ts...> &) {
                return *this;
            }

            template<size_t I, size_t N, class... Ts>
            message_builder &append_tuple(std::integral_constant<size_t, I>,
                                          std::integral_constant<size_t, N>
                                              e,
                                          std::tuple<Ts...> &xs) {
                append(std::move(std::get<I>(xs)));
                return append_tuple(std::integral_constant<size_t, I + 1> {}, e, xs);
            }

            template<class... Ts>
            message_builder &append_tuple(std::tuple<Ts...> xs) {
                return append_tuple(std::integral_constant<size_t, 0> {},
                                    std::integral_constant<size_t, sizeof...(Ts)> {}, xs);
            }

            /// Converts the buffer to an actual message object without
            /// invalidating this message builder (nor clearing it).
            message to_message() const;

            /// Converts the buffer to an actual message object and transfers
            /// ownership of the data to it, leaving this object in an invalid state.
            /// @warning Calling *any*  member function on this object afterwards
            ///          is undefined behavior (dereferencing a `nullptr`)
            message move_to_message();

            /// @copydoc message::extract
            message extract(message_handler f) const;

            /// @copydoc message::apply
            optional<message> apply(message_handler handler);

            /// Removes all elements from the buffer.
            void clear();

            /// Returns whether the buffer is empty.
            bool empty() const;

            /// Returns the number of elements in the buffer.
            size_t size() const;

        private:
            void init();

            message_builder &emplace(type_erased_value_ptr);

            intrusive_cow_ptr<detail::dynamic_message_data> data_;
        };

    }    // namespace mtl
}    // namespace nil
