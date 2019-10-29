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
#include <cstddef>
#include <cstdint>
#include <typeinfo>

#include <nil/mtl/detail/apply_args.hpp>
#include <nil/mtl/detail/pseudo_tuple.hpp>
#include <nil/mtl/detail/try_match.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/optional.hpp>
#include <nil/mtl/rtti_pair.hpp>
#include <nil/mtl/type_erased_value.hpp>
#include <nil/mtl/type_nr.hpp>

namespace nil {
    namespace mtl {

        /// Represents a tuple of type-erased values.
        class type_erased_tuple {
        public:
            // -- constructors, destructors, and assignment operators --------------------

            type_erased_tuple() = default;
            type_erased_tuple(const type_erased_tuple &) = default;
            type_erased_tuple &operator=(const type_erased_tuple &) = default;

            virtual ~type_erased_tuple();

            // -- pure virtual modifiers -------------------------------------------------

            /// Returns a mutable pointer to the element at position `pos`.
            virtual void *get_mutable(size_t pos) = 0;

            /// Load the content for the element at position `pos` from `source`.
            virtual error load(size_t pos, deserializer &source) = 0;

            // -- modifiers --------------------------------------------------------------

            /// Load the content for the tuple from `source`.
            virtual error load(deserializer &source);

            // -- pure virtual observers -------------------------------------------------

            /// Returns the size of this tuple.
            virtual size_t size() const noexcept = 0;

            /// Returns a type hint for the element types.
            virtual uint32_t type_token() const noexcept = 0;

            /// Returns the type number and `std::type_info` object for
            /// the element at position `pos`.
            virtual rtti_pair type(size_t pos) const noexcept = 0;

            /// Returns the element at position `pos`.
            virtual const void *get(size_t pos) const noexcept = 0;

            /// Returns a string representation of the element at position `pos`.
            virtual std::string stringify(size_t pos) const = 0;

            /// Returns a copy of the element at position `pos`.
            virtual type_erased_value_ptr copy(size_t pos) const = 0;

            /// Saves the element at position `pos` to `sink`.
            virtual error save(size_t pos, serializer &sink) const = 0;

            // -- observers --------------------------------------------------------------

            /// Returns whether multiple references to this tuple exist.
            /// The default implementation returns false.
            virtual bool shared() const noexcept;

            ///  Returns `size() == 0`.
            bool empty() const;

            /// Returns a string representation of the tuple.
            std::string stringify() const;

            /// Saves the content of the tuple to `sink`.
            virtual error save(serializer &sink) const;

            /// Checks whether the type of the stored value at position `pos`
            /// matches type number `n` and run-time type information `p`.
            bool matches(size_t pos, uint16_t nr, const std::type_info *ptr) const noexcept;

            // -- convenience functions --------------------------------------------------

            /// Returns the type number for the element at position `pos`.
            inline uint16_t type_nr(size_t pos) const noexcept {
                return type(pos).first;
            }

            /// Checks whether the type of the stored value matches `rtti`.
            inline bool matches(size_t pos, const rtti_pair &rtti) const noexcept {
                return matches(pos, rtti.first, rtti.second);
            }

            /// Convenience function for `*reinterpret_cast<const T*>(get())`.
            template<class T>
            const T &get_as(size_t pos) const {
                return *reinterpret_cast<const T *>(get(pos));
            }

            template<class T, size_t Pos>
            struct typed_index {};

            template<class T, size_t Pos>
            static constexpr typed_index<T, Pos> make_typed_index() {
                return {};
            }

            template<class T, size_t Pos>
            const T &get_as(typed_index<T, Pos>) const {
                return *reinterpret_cast<const T *>(get(Pos));
            }

            template<class... Ts, long... Is>
            std::tuple<const Ts &...> get_as_tuple(detail::type_list<Ts...>, detail::int_list<Is...>) const {
                return std::tuple<const Ts &...> {get_as<Ts>(Is)...};
                // return get_as<Ts>(Is)...;//(make_typed_index<Ts, Is>())...;
            }

            template<class... Ts>
            std::tuple<const Ts &...> get_as_tuple() const {
                return get_as_tuple(detail::type_list<Ts...> {}, typename detail::il_range<0, sizeof...(Ts)>::type {});
            }

            /// Convenience function for `*reinterpret_cast<T*>(get_mutable())`.
            template<class T>
            T &get_mutable_as(size_t pos) {
                return *reinterpret_cast<T *>(get_mutable(pos));
            }

            /// Convenience function for moving a value out of the tuple if it is
            /// unshared. Returns a copy otherwise.
            template<class T>
            T move_if_unshared(size_t pos) {
                if (shared())
                    return get_as<T>(pos);
                return std::move(get_mutable_as<T>(pos));
            }

            /// Returns `true` if the element at `pos` matches `T`.
            template<class T>
            bool match_element(size_t pos) const noexcept {
                MTL_ASSERT(pos < size());
                auto x = detail::meta_element_factory<T>::create();
                return x.fun(x, *this, pos);
            }

            /// Returns `true` if the pattern `Ts...` matches the content of this tuple.
            template<class... Ts>
            bool match_elements() const noexcept {
                detail::type_list<Ts...> tk;
                return match_elements(tk);
            }

            template<class F>
            auto apply(F fun) -> optional<typename detail::get_callable_trait<F>::result_type> {
                using trait = typename detail::get_callable_trait<F>::type;
                detail::type_list<typename trait::result_type> result_token;
                typename trait::arg_types args_token;
                return apply(fun, result_token, args_token);
            }

            /// @private
            template<class T, class... Ts>
            bool match_elements(detail::type_list<T, Ts...>) const noexcept {
                detail::meta_elements<detail::type_list<T, Ts...>> xs;
                return detail::try_match(*this, &xs.arr[0], 1 + sizeof...(Ts));
            }

            /// @private
            inline bool match_elements(detail::type_list<>) const noexcept {
                return empty();
            }

        private:
            template<class F, class R, class... Ts>
            optional<R> apply(F &fun, detail::type_list<R>, detail::type_list<Ts...> tk) {
                if (!match_elements<Ts...>())
                    return none;
                detail::pseudo_tuple<typename std::decay<Ts>::type...> xs {*this};
                return detail::apply_args(fun, detail::get_indices(tk), xs);
            }

            template<class F, class... Ts>
            optional<void> apply(F &fun, detail::type_list<void>, detail::type_list<Ts...> tk) {
                if (!match_elements<Ts...>())
                    return none;
                detail::pseudo_tuple<typename std::decay<Ts>::type...> xs {*this};
                detail::apply_args(fun, detail::get_indices(tk), xs);
                return unit;
            }
        };

        /// @relates type_erased_tuple
        template<class Processor>
        typename std::enable_if<Processor::reads_state>::type serialize(Processor &proc, type_erased_tuple &x) {
            x.save(proc);
        }

        /// @relates type_erased_tuple
        template<class Processor>
        typename std::enable_if<Processor::writes_state>::type serialize(Processor &proc, type_erased_tuple &x) {
            x.load(proc);
        }

        /// @relates type_erased_tuple
        inline std::string to_string(const type_erased_tuple &x) {
            return x.stringify();
        }

        /// @relates type_erased_tuple
        /// Dummy objects representing empty tuples.
        class empty_type_erased_tuple : public type_erased_tuple {
        public:
            empty_type_erased_tuple() = default;

            ~empty_type_erased_tuple() override;

            void *get_mutable(size_t pos) override;

            error load(size_t pos, deserializer &source) override;

            size_t size() const noexcept override;

            uint32_t type_token() const noexcept override;

            rtti_pair type(size_t pos) const noexcept override;

            const void *get(size_t pos) const noexcept override;

            std::string stringify(size_t pos) const override;

            type_erased_value_ptr copy(size_t pos) const override;

            error save(size_t pos, serializer &sink) const override;
        };

    }    // namespace mtl
}    // namespace nil
