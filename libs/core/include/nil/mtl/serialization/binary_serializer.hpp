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

#include <cstddef>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <nil/mtl/byte.hpp>
#include <nil/mtl/byte_buffer.hpp>
#include <nil/mtl/error_code.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/read_inspector.hpp>
#include <nil/mtl/sec.hpp>
#include <nil/mtl/span.hpp>

namespace nil {
    namespace mtl {
        class binary_serializer : public read_inspector<binary_serializer> {
        public:
            // -- member types -----------------------------------------------------------

            using result_type = error_code<sec>;

            using container_type = byte_buffer;

            using value_type = byte;

            // -- constructors, destructors, and assignment operators --------------------

            binary_serializer(actor_system &sys, byte_buffer &buf) noexcept;

            binary_serializer(execution_unit *ctx, byte_buffer &buf) noexcept :
                buf_(buf), write_pos_(buf.size()), context_(ctx) {
                // nop
            }

            binary_serializer(const binary_serializer &) = delete;

            binary_serializer &operator=(const binary_serializer &) = delete;

            // -- properties -------------------------------------------------------------

            /// Returns the current execution unit.
            execution_unit *context() const noexcept {
                return context_;
            }

            byte_buffer &buf() noexcept {
                return buf_;
            }

            const byte_buffer &buf() const noexcept {
                return buf_;
            }

            size_t write_pos() const noexcept {
                return write_pos_;
            }

            // -- position management ----------------------------------------------------

            /// Sets the write position to `offset`.
            /// @pre `offset <= buf.size()`
            void seek(size_t offset) noexcept {
                write_pos_ = offset;
            }

            /// Jumps `num_bytes` forward. Resizes the buffer (filling it with zeros)
            /// when skipping past the end.
            void skip(size_t num_bytes);

            // -- interface functions ----------------------------------------------------

            error_code<sec> begin_object(uint16_t nr, string_view name);

            error_code<sec> end_object();

            error_code<sec> begin_sequence(size_t list_size);

            error_code<sec> end_sequence();

            void apply(uint8_t x);

            void apply(uint16_t x);

            void apply(uint32_t x);

            void apply(uint64_t x);

            void apply(float x);

            void apply(double x);

            void apply(long double x);

            void apply(string_view x);

            void apply(const std::u16string &x);

            void apply(const std::u32string &x);

            void apply(span<const byte> x);

            template<class T>
            typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value>::type apply(T x) {
                using unsigned_type = std::make_unsigned_t<T>;
                using squashed_unsigned_type = detail::squashed_int_t<unsigned_type>;
                return apply(static_cast<squashed_unsigned_type>(x));
            }

            template<class Enum>
            typename std::enable_if<std::is_enum<Enum>::value>::type apply(Enum x) {
                return apply(static_cast<std::underlying_type_t<Enum>>(x));
            }

            void apply(const std::vector<bool> &x);

        private:
            /// Stores the serialized output.
            byte_buffer &buf_;

            /// Stores the current offset for writing.
            size_t write_pos_;

            /// Provides access to the ::proxy_registry and to the ::actor_system.
            execution_unit *context_;
        };
    }    // namespace mtl
}    // namespace nil
