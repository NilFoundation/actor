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

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace nil {
    namespace mtl {
        namespace intrusive {

            // A forward iterator for intrusive lists.
            template<class T>
            class forward_iterator {
            public:
                // -- member types -----------------------------------------------------------

                using difference_type = std::ptrdiff_t;

                using value_type = T;

                using pointer = value_type *;

                using const_pointer = const value_type *;

                using reference = value_type &;

                using const_reference = const value_type &;

                using node_type = typename std::conditional<std::is_const<T>::value, const typename T::node_type,
                                                            typename T::node_type>::type;

                using node_pointer = node_type *;

                using iterator_category = std::forward_iterator_tag;

                // -- member variables -------------------------------------------------------

                node_pointer ptr;

                // -- constructors, destructors, and assignment operators --------------------

                constexpr forward_iterator(node_pointer init = nullptr) : ptr(init) {
                    // nop
                }

                forward_iterator(const forward_iterator &) = default;

                forward_iterator &operator=(const forward_iterator &) = default;

                // -- convenience functions --------------------------------------------------

                forward_iterator next() {
                    return ptr->next;
                }

                // -- operators --------------------------------------------------------------

                forward_iterator &operator++() {
                    ptr = promote(ptr->next);
                    return *this;
                }

                forward_iterator operator++(int) {
                    forward_iterator res = *this;
                    ptr = promote(ptr->next);
                    return res;
                }

                reference operator*() {
                    return *promote(ptr);
                }

                const_reference operator*() const {
                    return *promote(ptr);
                }

                pointer operator->() {
                    return promote(ptr);
                }

                const_pointer operator->() const {
                    return promote(ptr);
                }
            };

            /// @relates forward_iterator
            template<class T>
            bool operator==(const forward_iterator<T> &x, const forward_iterator<T> &y) {
                return x.ptr == y.ptr;
            }

            /// @relates forward_iterator
            template<class T>
            bool operator!=(const forward_iterator<T> &x, const forward_iterator<T> &y) {
                return x.ptr != y.ptr;
            }

        }    // namespace intrusive
    }        // namespace mtl
}    // namespace nil
