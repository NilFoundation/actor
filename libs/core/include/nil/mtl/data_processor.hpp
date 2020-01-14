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

#include <chrono>
#include <string>
#include <cstdint>
#include <cstddef>    // size_t
#include <type_traits>
#include <iterator>

#include <boost/integer.hpp>

#include <nil/mtl/fwd.hpp>
#include <nil/mtl/atom.hpp>
#include <nil/mtl/error.hpp>
#include <nil/mtl/timestamp.hpp>
#include <nil/mtl/allowed_unsafe_message_type.hpp>

#include <nil/mtl/meta/annotation.hpp>
#include <nil/mtl/meta/save_callback.hpp>
#include <nil/mtl/meta/load_callback.hpp>

#include <nil/mtl/detail/type_list.hpp>
#include <nil/mtl/detail/apply_args.hpp>
#include <nil/mtl/detail/delegate_serialize.hpp>

namespace nil {
    namespace mtl {

        /// A data processor translates an object into a format that can be
        /// stored or vice versa. A data processor can be either in saving
        /// or loading mode.
        template<class Derived>
        class data_processor {
        public:
            // -- member types -----------------------------------------------------------

            /// Return type for `operator()`.
            using result_type = error;

            /// List of builtin types for data processors.
            using builtin_t = detail::type_list<int8_t,
                                                uint8_t,
                                                int16_t,
                                                uint16_t,
                                                int32_t,
                                                uint32_t,
                                                int64_t,
                                                uint64_t,
                                                float,
                                                double,
                                                long double,
                                                std::string,
                                                std::u16string,
                                                std::u32string>;

            // -- constructors, destructors, and assignment operators --------------------

            data_processor(const data_processor &) = delete;
            data_processor &operator=(const data_processor &) = delete;

            data_processor(execution_unit *ctx = nullptr) : context_(ctx) {
                // nop
            }

            virtual ~data_processor() {
                // nop
            }

            // -- pure virtual functions -------------------------------------------------

            /// Begins processing of an object. Saves the type information
            /// to the underlying storage when in saving mode, otherwise
            /// extracts them and sets both arguments accordingly.
            virtual error begin_object(uint16_t &typenr, std::string &name) = 0;

            /// Ends processing of an object.
            virtual error end_object() = 0;

            /// Begins processing of a sequence. Saves the size
            /// to the underlying storage when in saving mode, otherwise
            /// sets `num` accordingly.
            virtual error begin_sequence(size_t &num) = 0;

            /// Ends processing of a sequence.
            virtual error end_sequence() = 0;

            // -- getter -----------------------------------------------------------------

            /// Returns the actor system associated to this data processor.
            execution_unit *context() {
                return context_;
            }

            // -- apply functions --------------------------------------------------------

            /// Applies this processor to an arithmetic type.
            template<class T>
            typename std::enable_if<std::is_floating_point<T>::value, error>::type apply(T &x) {
                static constexpr auto tlindex = detail::tl_index_of<builtin_t, T>::value;
                static_assert(tlindex >= 0, "T not recognized as builtin type");
                return apply_impl(x);
            }

            template<class T>
            typename std::enable_if<std::is_integral<T>::value && !std::is_same<bool, T>::value, error>::type
                apply(T &x) {
                using type = typename std::conditional<std::is_signed<T>::value,
                                                       typename boost::int_t<sizeof(T) * CHAR_BIT>::exact,
                                                       typename boost::uint_t<sizeof(T) * CHAR_BIT>::exact>::type;
                return apply_impl(reinterpret_cast<type &>(x));
            }

            error apply(std::string &x) {
                return apply_impl(x);
            }

            error apply(std::u16string &x) {
                return apply_impl(x);
            }

            error apply(std::u32string &x) {
                return apply_impl(x);
            }

            template<class D, atom_value V>
            static error apply_atom_constant(D &self, atom_constant<V> &) {
                static_assert(!D::writes_state, "cannot deserialize an atom_constant");
                auto x = V;
                return self.apply(x);
            }

            template<atom_value V>
            error apply(atom_constant<V> &x) {
                return apply_atom_constant(dref(), x);
            }

            /// Serializes enums using the underlying type
            /// if no user-defined serialization is defined.
            template<class T>
            typename std::enable_if<std::is_enum<T>::value && !detail::has_serialize<T>::value, error>::type
                apply(T &x) {
                using underlying = typename std::underlying_type<T>::type;
                struct {
                    void operator()(T &lhs, underlying &rhs) const {
                        lhs = static_cast<T>(rhs);
                    }
                    void operator()(underlying &lhs, T &rhs) const {
                        lhs = static_cast<underlying>(rhs);
                    }
                } assign;
                underlying tmp;
                return convert_apply(dref(), x, tmp, assign);
            }

            /// Applies this processor to an empty type.
            template<class T>
            typename std::enable_if<std::is_empty<T>::value && !detail::is_inspectable<Derived, T>::value, error>::type
                apply(T &) {
                return none;
            }

            error apply(bool &x) {
                struct {
                    void operator()(bool &lhs, uint8_t &rhs) const {
                        lhs = rhs != 0;
                    }
                    void operator()(uint8_t &lhs, bool &rhs) const {
                        lhs = rhs ? 1 : 0;
                    }
                } assign;
                uint8_t tmp;
                return convert_apply(dref(), x, tmp, assign);
            }

            // Special case to avoid using 1 byte per bool
            error apply(std::vector<bool> &x) {
                auto len = x.size();
                auto err = begin_sequence(len);
                if (err || len == 0)
                    return err;
                struct {
                    size_t len;
                    void operator()(std::vector<bool> &lhs, std::vector<uint8_t> &rhs) const {
                        lhs.resize(len, false);
                        size_t cpt = 0;
                        for (auto v : rhs) {
                            for (int k = 0; k < 8; ++k) {
                                lhs[cpt] = ((v & (1 << k)) != 0);
                                if (++cpt >= len)
                                    return;
                            }
                        }
                    }
                    void operator()(std::vector<uint8_t> &lhs, std::vector<bool> &rhs) const {
                        size_t k = 0;
                        lhs.resize((rhs.size() - 1) / 8 + 1, 0);
                        for (bool b : rhs) {
                            if (b)
                                lhs[k / 8] |= (1 << (k % 8));
                            ++k;
                        }
                    }
                } assign;
                assign.len = len;
                std::vector<uint8_t> tmp;
                return convert_apply(dref(), x, tmp, assign);
            }

            template<class T>
            error consume_range(T &xs) {
                for (auto &x : xs) {
                    using value_type = typename std::remove_reference<decltype(x)>::type;
                    using mutable_type = typename std::remove_const<value_type>::type;
                    if (auto err = apply_derived(const_cast<mutable_type &>(x)))
                        return err;
                }
                return none;
            }

            /// Converts each element in `xs` to `U` before calling `apply`.
            template<class U, class T>
            error consume_range_c(T &xs) {
                for (U x : xs) {
                    if (auto err = apply_derived(x))
                        return err;
                }
                return none;
            }

            template<class T>
            error fill_range(T &xs, size_t num_elements) {
                xs.clear();
                auto insert_iter = std::inserter(xs, xs.end());
                for (size_t i = 0; i < num_elements; ++i) {
                    typename std::remove_const<typename T::value_type>::type x;
                    if (auto err = apply_derived(x))
                        return err;
                    *insert_iter++ = std::move(x);
                }
                return none;
            }

            /// Loads elements from type `U` before inserting to `xs`.
            template<class U, class T>
            error fill_range_c(T &xs, size_t num_elements) {
                xs.clear();
                auto insert_iter = std::inserter(xs, xs.end());
                for (size_t i = 0; i < num_elements; ++i) {
                    U x;
                    if (auto err = apply_derived(x))
                        return err;
                    *insert_iter++ = std::move(x);
                }
                return none;
            }
            // Applies this processor as Derived to `xs` in saving mode.
            template<class D, class T>
            static typename std::enable_if<D::reads_state && !detail::is_byte_sequence<T>::value, error>::type
                apply_sequence(D &self, T &xs) {
                auto s = xs.size();
                return error::eval([&] { return self.begin_sequence(s); },
                                   [&] { return self.consume_range(xs); },
                                   [&] { return self.end_sequence(); });
            }

            // Applies this processor as Derived to `xs` in loading mode.
            template<class D, class T>
            static typename std::enable_if<!D::reads_state && !detail::is_byte_sequence<T>::value, error>::type
                apply_sequence(D &self, T &xs) {
                size_t s;
                return error::eval([&] { return self.begin_sequence(s); },
                                   [&] { return self.fill_range(xs, s); },
                                   [&] { return self.end_sequence(); });
            }

            // Optimized saving for contiguous byte sequences.
            template<class D, class T>
            static typename std::enable_if<D::reads_state && detail::is_byte_sequence<T>::value, error>::type
                apply_sequence(D &self, T &xs) {
                auto s = xs.size();
                return error::eval([&] { return self.begin_sequence(s); },
                                   [&] { return s > 0 ? self.apply_raw(xs.size(), &xs[0]) : none; },
                                   [&] { return self.end_sequence(); });
            }

            // Optimized loading for contiguous byte sequences.
            template<class D, class T>
            static typename std::enable_if<!D::reads_state && detail::is_byte_sequence<T>::value, error>::type
                apply_sequence(D &self, T &xs) {
                size_t s;
                return error::eval([&] { return self.begin_sequence(s); },
                                   [&] {
                                       xs.resize(s);
                                       return s > 0 ? self.apply_raw(s, &xs[0]) : none;
                                   },
                                   [&] { return self.end_sequence(); });
            }

            /// Applies this processor to a sequence of values.
            template<class T>
            typename std::enable_if<detail::is_iterable<T>::value && !detail::has_serialize<T>::value &&
                                        !detail::is_inspectable<Derived, T>::value,
                                    error>::type
                apply(T &xs) {
                return apply_sequence(dref(), xs);
            }

            /// Applies this processor to an array.
            template<class T, size_t S>
            typename std::enable_if<detail::is_serializable<T>::value, error>::type apply(std::array<T, S> &xs) {
                return consume_range(xs);
            }

            /// Applies this processor to an array.
            template<class T, size_t S>
            typename std::enable_if<detail::is_serializable<T>::value, error>::type apply(T (&xs)[S]) {
                return consume_range(xs);
            }

            template<class F, class S>
            typename std::enable_if<detail::is_serializable<typename std::remove_const<F>::type>::value &&
                                        detail::is_serializable<S>::value,
                                    error>::type
                apply(std::pair<F, S> &xs) {
                using t0 = typename std::remove_const<F>::type;
                // This cast allows the data processor to cope with
                // `pair<const K, V>` value types used by `std::map`.
                return error::eval([&] { return apply_derived(const_cast<t0 &>(xs.first)); },
                                   [&] { return apply_derived(xs.second); });
            }

            template<class... Ts>
            typename std::enable_if<detail::conjunction<detail::is_serializable<Ts>::value...>::value, error>::type
                apply(std::tuple<Ts...> &xs) {
                // apply_helper f{*this};
                return detail::apply_args(*this, detail::get_indices(xs), xs);
            }

            template<class T>
            typename std::enable_if<!std::is_empty<T>::value && detail::has_serialize<T>::value, error>::type
                apply(T &x) {
                // throws on error
                detail::delegate_serialize(dref(), x);
                return none;
            }

            template<class Rep, class Period>
            typename std::enable_if<std::is_integral<Rep>::value, error>::type
                apply(std::chrono::duration<Rep, Period> &x) {
                using duration_type = std::chrono::duration<Rep, Period>;
                // always save/store durations as int64_t to work around possibly
                // different integer types on different plattforms for standard typedefs
                struct {
                    void operator()(duration_type &lhs, Rep &rhs) const {
                        duration_type tmp {rhs};
                        lhs = tmp;
                    }
                    void operator()(Rep &lhs, duration_type &rhs) const {
                        lhs = rhs.count();
                    }
                } assign;
                Rep tmp;
                return convert_apply(dref(), x, tmp, assign);
            }

            template<class Rep, class Period>
            typename std::enable_if<std::is_floating_point<Rep>::value, error>::type
                apply(std::chrono::duration<Rep, Period> &x) {
                using duration_type = std::chrono::duration<Rep, Period>;
                // always save/store floating point durations
                // as doubles for the same reason as above
                struct {
                    void operator()(duration_type &lhs, double &rhs) const {
                        duration_type tmp {rhs};
                        lhs = tmp;
                    }
                    void operator()(double &lhs, duration_type &rhs) const {
                        lhs = rhs.count();
                    }
                } assign;
                double tmp;
                return convert_apply(dref(), x, tmp, assign);
            }

            template<class Duration>
            error apply(std::chrono::time_point<std::chrono::system_clock, Duration> &t) {
                if (Derived::reads_state) {
                    auto dur = t.time_since_epoch();
                    return apply(dur);
                }
                if (Derived::writes_state) {
                    Duration dur {};
                    auto e = apply(dur);
                    t = std::chrono::time_point<std::chrono::system_clock, Duration> {dur};
                    return e;
                }
            }

            /// Applies this processor to a raw block of data of size `num_bytes`.
            virtual error apply_raw(size_t num_bytes, void *data) = 0;

            template<class T>
            typename std::enable_if<detail::is_inspectable<Derived, T>::value && !detail::has_serialize<T>::value,
                                    decltype(inspect(std::declval<Derived &>(), std::declval<T &>()))>::type
                apply(T &x) {
                return inspect(dref(), x);
            }

            // -- operator() -------------------------------------------------------------

            inline error operator()() {
                return none;
            }

            template<class F, class... Ts>
            error operator()(meta::save_callback_t<F> x, Ts &&... xs) {
                error e;
                if (Derived::reads_state)
                    e = x.fun();
                return e ? e : (*this)(std::forward<Ts>(xs)...);
            }

            template<class F, class... Ts>
            error operator()(meta::load_callback_t<F> x, Ts &&... xs) {
                error e;
                if (Derived::writes_state)
                    e = x.fun();
                return e ? e : (*this)(std::forward<Ts>(xs)...);
            }

            template<class... Ts>
            error operator()(const meta::annotation &, Ts &&... xs) {
                return (*this)(std::forward<Ts>(xs)...);
            }

            template<class T, class... Ts>
            typename std::enable_if<is_allowed_unsafe_message_type<T>::value, error>::type operator()(const T &,
                                                                                                      Ts &&... xs) {
                return (*this)(std::forward<Ts>(xs)...);
            }

            template<class T, class... Ts>
            typename std::enable_if<!meta::is_annotation<T>::value && !is_allowed_unsafe_message_type<T>::value,
                                    error>::type
                operator()(T &&x, Ts &&... xs) {
                static_assert(Derived::reads_state || (!std::is_rvalue_reference<T &&>::value &&
                                                       !std::is_const<typename std::remove_reference<T>::type>::value),
                              "a loading inspector requires mutable lvalue references");
                if (auto err = apply(deconst(x)))
                    return err;
                return apply_derived(std::forward<Ts>(xs)...);
            }

        protected:
            virtual error apply_impl(int8_t &) = 0;

            virtual error apply_impl(uint8_t &) = 0;

            virtual error apply_impl(int16_t &) = 0;

            virtual error apply_impl(uint16_t &) = 0;

            virtual error apply_impl(int32_t &) = 0;

            virtual error apply_impl(uint32_t &) = 0;

            virtual error apply_impl(int64_t &) = 0;

            virtual error apply_impl(uint64_t &) = 0;

            virtual error apply_impl(float &) = 0;

            virtual error apply_impl(double &) = 0;

            virtual error apply_impl(long double &) = 0;

            virtual error apply_impl(std::string &) = 0;

            virtual error apply_impl(std::u16string &) = 0;

            virtual error apply_impl(std::u32string &) = 0;

        private:
            template<class T>
            T &deconst(const T &x) {
                return const_cast<T &>(x);
            }

            template<class D, class T, class U, class F>
            static typename std::enable_if<D::reads_state, error>::type
                convert_apply(D &self, T &x, U &storage, F assign) {
                assign(storage, x);
                return self(storage);
            }

            template<class D, class T, class U, class F>
            static typename std::enable_if<!D::reads_state, error>::type
                convert_apply(D &self, T &x, U &storage, F assign) {
                if (auto err = self(storage))
                    return err;
                assign(x, storage);
                return none;
            }

            // Returns a reference to the derived type.
            Derived &dref() {
                return *static_cast<Derived *>(this);
            }

            // Applies `xs...` to `dref()`.
            template<class... Ts>
            error apply_derived(Ts &&... xs) {
                return dref()(std::forward<Ts>(xs)...);
            }

            execution_unit *context_;
        };

    }    // namespace mtl
}    // namespace nil
