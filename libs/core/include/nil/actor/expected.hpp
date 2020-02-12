//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt or
// http://opensource.org/licenses/BSD-3-Clause
//---------------------------------------------------------------------------//

#pragma once

#include <nil/actor/config.hpp>

#include <new>
#include <memory>
#include <ostream>
#include <type_traits>

#include <nil/actor/deep_to_string.hpp>
#include <nil/actor/error.hpp>
#include <nil/actor/unifyn.hpp>
#include <nil/actor/unit.hpp>

namespace nil {
    namespace actor {

        /// Helper class to construct an `expected<T>` that represents no error.
        /// @relates expected
        struct no_error_t {};

        /// The only instance of ::no_error_t.
        /// @relates expected
        constexpr no_error_t no_error = no_error_t {};

        /// Represents the result of a computation which can either complete
        /// successfully with an instance of type `T` or fail with an `error`.
        /// @tparam T The type of the result.
        template<typename T>
        class expected {
        public:
            // -- member types -----------------------------------------------------------

            using value_type = T;

            // -- static member variables ------------------------------------------------

            /// Stores whether move construct and move assign never throw.
            static constexpr bool nothrow_move =
                std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value;

            /// Stores whether copy construct and copy assign never throw.
            static constexpr bool nothrow_copy =
                std::is_nothrow_copy_constructible<T>::value && std::is_nothrow_copy_assignable<T>::value;

            // -- constructors, destructors, and assignment operators --------------------

            template<class U>
            expected(U x, typename std::enable_if<std::is_convertible<U, T>::value>::type * = nullptr) :
                engaged_(true) {
                new (std::addressof(value_)) T(std::move(x));
            }

            expected(T &&x) noexcept(nothrow_move) : engaged_(true) {
                new (std::addressof(value_)) T(std::move(x));
            }

            expected(const T &x) noexcept(nothrow_copy) : engaged_(true) {
                new (std::addressof(value_)) T(x);
            }

            expected(nil::actor::error e) noexcept : engaged_(false) {
                new (std::addressof(error_)) nil::actor::error {std::move(e)};
            }

            expected(no_error_t) noexcept : engaged_(false) {
                new (std::addressof(error_)) nil::actor::error {};
            }

            expected(const expected &other) noexcept(nothrow_copy) {
                construct(other);
            }

            template<class Code, class E = enable_if_has_make_error_t<Code>>
            expected(Code code) : engaged_(false) {
                new (std::addressof(error_)) nil::actor::error(make_error(code));
            }

            expected(expected &&other) noexcept(nothrow_move) {
                construct(std::move(other));
            }

            ~expected() {
                destroy();
            }

            expected &operator=(const expected &other) noexcept(nothrow_copy) {
                if (engaged_ && other.engaged_)
                    value_ = other.value_;
                else if (!engaged_ && !other.engaged_)
                    error_ = other.error_;
                else {
                    destroy();
                    construct(other);
                }
                return *this;
            }

            expected &operator=(expected &&other) noexcept(nothrow_move) {
                if (engaged_ && other.engaged_)
                    value_ = std::move(other.value_);
                else if (!engaged_ && !other.engaged_)
                    error_ = std::move(other.error_);
                else {
                    destroy();
                    construct(std::move(other));
                }
                return *this;
            }

            expected &operator=(const T &x) noexcept(nothrow_copy) {
                if (engaged_) {
                    value_ = x;
                } else {
                    destroy();
                    engaged_ = true;
                    new (std::addressof(value_)) T(x);
                }
                return *this;
            }

            expected &operator=(T &&x) noexcept(nothrow_move) {
                if (engaged_) {
                    value_ = std::move(x);
                } else {
                    destroy();
                    engaged_ = true;
                    new (std::addressof(value_)) T(std::move(x));
                }
                return *this;
            }

            template<class U>
            typename std::enable_if<std::is_convertible<U, T>::value, expected &>::type operator=(U x) {
                return *this = T {std::move(x)};
            }

            expected &operator=(nil::actor::error e) noexcept {
                if (!engaged_)
                    error_ = std::move(e);
                else {
                    destroy();
                    engaged_ = false;
                    new (std::addressof(error_)) nil::actor::error(std::move(e));
                }
                return *this;
            }

            template<class Code>
            enable_if_has_make_error_t<Code, expected &> operator=(Code code) {
                return *this = make_error(code);
            }

            // -- modifiers --------------------------------------------------------------

            /// @copydoc cvalue
            T &value() noexcept {
                ACTOR_ASSERT(engaged_);
                return value_;
            }

            /// @copydoc cvalue
            T &operator*() noexcept {
                return value();
            }

            /// @copydoc cvalue
            T *operator->() noexcept {
                return &value();
            }

            /// @copydoc cerror
            nil::actor::error &error() noexcept {
                ACTOR_ASSERT(!engaged_);
                return error_;
            }

            // -- observers --------------------------------------------------------------

            /// Returns the contained value.
            /// @pre `engaged() == true`.
            const T &cvalue() const noexcept {
                ACTOR_ASSERT(engaged_);
                return value_;
            }

            /// @copydoc cvalue
            const T &value() const noexcept {
                ACTOR_ASSERT(engaged_);
                return value_;
            }

            /// @copydoc cvalue
            const T &operator*() const noexcept {
                return value();
            }

            /// @copydoc cvalue
            const T *operator->() const noexcept {
                return &value();
            }

            /// @copydoc engaged
            explicit operator bool() const noexcept {
                return engaged();
            }

            /// Returns `true` if the object holds a value (is engaged).
            bool engaged() const noexcept {
                return engaged_;
            }

            /// Returns the contained error.
            /// @pre `engaged() == false`.
            const nil::actor::error &cerror() const noexcept {
                ACTOR_ASSERT(!engaged_);
                return error_;
            }

            /// @copydoc cerror
            const nil::actor::error &error() const noexcept {
                ACTOR_ASSERT(!engaged_);
                return error_;
            }

        private:
            void construct(expected &&other) noexcept(nothrow_move) {
                if (other.engaged_)
                    new (std::addressof(value_)) T(std::move(other.value_));
                else
                    new (std::addressof(error_)) nil::actor::error(std::move(other.error_));
                engaged_ = other.engaged_;
            }

            void construct(const expected &other) noexcept(nothrow_copy) {
                if (other.engaged_)
                    new (std::addressof(value_)) T(other.value_);
                else
                    new (std::addressof(error_)) nil::actor::error(other.error_);
                engaged_ = other.engaged_;
            }

            void destroy() {
                if (engaged_)
                    value_.~T();
                else
                    error_.~error();
            }

            bool engaged_;

            union {
                T value_;
                nil::actor::error error_;
            };
        };

        /// @relates expected
        template<class T>
        auto operator==(const expected<T> &x, const expected<T> &y) -> decltype(*x == *y) {
            return x && y ? *x == *y : (!x && !y ? x.error() == y.error() : false);
        }

        /// @relates expected
        template<class T, class U>
        auto operator==(const expected<T> &x, const U &y) -> decltype(*x == y) {
            return x ? *x == y : false;
        }

        /// @relates expected
        template<class T, class U>
        auto operator==(const T &x, const expected<U> &y) -> decltype(x == *y) {
            return y == x;
        }

        /// @relates expected
        template<class T>
        bool operator==(const expected<T> &x, const error &y) {
            return x ? false : x.error() == y;
        }

        /// @relates expected
        template<class T>
        bool operator==(const error &x, const expected<T> &y) {
            return y == x;
        }

        /// @relates expected
        template<class T, class E>
        enable_if_has_make_error_t<E, bool> operator==(const expected<T> &x, E y) {
            return x == make_error(y);
        }

        /// @relates expected
        template<class T, class E>
        enable_if_has_make_error_t<E, bool> operator==(E x, const expected<T> &y) {
            return y == make_error(x);
        }

        /// @relates expected
        template<class T>
        auto operator!=(const expected<T> &x, const expected<T> &y) -> decltype(*x == *y) {
            return !(x == y);
        }

        /// @relates expected
        template<class T, class U>
        auto operator!=(const expected<T> &x, const U &y) -> decltype(*x == y) {
            return !(x == y);
        }

        /// @relates expected
        template<class T, class U>
        auto operator!=(const T &x, const expected<U> &y) -> decltype(x == *y) {
            return !(x == y);
        }

        /// @relates expected
        template<class T>
        bool operator!=(const expected<T> &x, const error &y) {
            return !(x == y);
        }

        /// @relates expected
        template<class T>
        bool operator!=(const error &x, const expected<T> &y) {
            return !(x == y);
        }

        /// @relates expected
        template<class T, class E>
        enable_if_has_make_error_t<E, bool> operator!=(const expected<T> &x, E y) {
            return !(x == y);
        }

        /// @relates expected
        template<class T, class E>
        enable_if_has_make_error_t<E, bool> operator!=(E x, const expected<T> &y) {
            return !(x == y);
        }

        /// The pattern `expected<void>` shall be used for functions that may generate
        /// an error but would otherwise return `bool`.
        template<>
        class expected<void> {
        public:
            expected() = default;

            expected(unit_t) noexcept {
                // nop
            }

            expected(no_error_t) noexcept {
                // nop
            }

            expected(nil::actor::error e) noexcept : error_(std::move(e)) {
                // nop
            }

            expected(const expected &other) noexcept : error_(other.error_) {
                // nop
            }

            expected(expected &&other) noexcept : error_(std::move(other.error_)) {
                // nop
            }

            template<class Code, class E = enable_if_has_make_error_t<Code>>
            expected(Code code) : error_(make_error(code)) {
                // nop
            }

            expected &operator=(const expected &other) = default;

            expected &operator=(expected &&other) noexcept {
                error_ = std::move(other.error_);
                return *this;
            }

            explicit operator bool() const {
                return !error_;
            }

            const nil::actor::error &error() const {
                return error_;
            }

        private:
            nil::actor::error error_;
        };

        /// @relates expected
        inline bool operator==(const expected<void> &x, const expected<void> &y) {
            return (x && y) || (!x && !y && x.error() == y.error());
        }

        /// @relates expected
        inline bool operator!=(const expected<void> &x, const expected<void> &y) {
            return !(x == y);
        }

        template<>
        class expected<unit_t> : public expected<void> {
        public:
            using expected<void>::expected;
        };

        template<class T>
        std::string to_string(const expected<T> &x) {
            if (x)
                return deep_to_string(*x);
            return "!" + to_string(x.error());
        }

        inline std::string to_string(const expected<void> &x) {
            if (x)
                return "unit";
            return "!" + to_string(x.error());
        }

    }    // namespace actor
}    // namespace nil

namespace std {

    template<class T>
    auto operator<<(ostream &oss, const nil::actor::expected<T> &x) -> decltype(oss << *x) {
        if (x)
            oss << *x;
        else
            oss << "!" << to_string(x.error());
        return oss;
    }

}    // namespace std
