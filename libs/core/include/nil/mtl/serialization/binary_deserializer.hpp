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

#include <cstddef>
#include <cstdint>
#include <vector>

#include <nil/mtl/byte.hpp>
#include <nil/mtl/serialization/deserializer.hpp>
#include <nil/mtl/span.hpp>

namespace nil {
    namespace mtl {

        /// Implements the deserializer interface with a binary serialization protocol.
        class binary_deserializer final : public deserializer {
        public:
            // -- member types -----------------------------------------------------------

            using super = deserializer;

            // -- constructors, destructors, and assignment operators --------------------

            binary_deserializer(actor_system &sys, span<const byte> bytes);

            binary_deserializer(execution_unit *ctx, span<const byte> bytes);

            template<class T>
            binary_deserializer(actor_system &sys, const std::vector<T> &buf) :
                binary_deserializer(sys, as_bytes(make_span(buf))) {
                // nop
            }

            template<class T>
            binary_deserializer(execution_unit *ctx, const std::vector<T> &buf) :
                binary_deserializer(ctx, as_bytes(make_span(buf))) {
                // nop
            }

            binary_deserializer(actor_system &sys, const char *buf, size_t buf_size);

            binary_deserializer(execution_unit *ctx, const char *buf, size_t buf_size);

            // -- overridden member functions --------------------------------------------

            error begin_object(uint16_t &typenr, std::string &name) override;

            error end_object() override;

            error begin_sequence(size_t &list_size) override;

            error end_sequence() override;

            error apply_raw(size_t num_bytes, void *data) override;

            // -- properties -------------------------------------------------------------

            /// Returns how many bytes are still available to read.
            size_t remaining() const noexcept {
                return static_cast<size_t>(end_ - current_);
            }

            /// Returns the remaining bytes.
            span<const byte> remainder() const noexcept {
                return make_span(current_, end_);
            }

            /// Jumps `num_bytes` forward.
            /// @pre `num_bytes <= remaining()`
            void skip(size_t num_bytes);

            /// Assings a new input.
            void reset(span<const byte> bytes);

        protected:
            error apply_impl(int8_t &) override;

            error apply_impl(uint8_t &) override;

            error apply_impl(int16_t &) override;

            error apply_impl(uint16_t &) override;

            error apply_impl(int32_t &) override;

            error apply_impl(uint32_t &) override;

            error apply_impl(int64_t &) override;

            error apply_impl(uint64_t &) override;

            error apply_impl(float &) override;

            error apply_impl(double &) override;

            error apply_impl(long double &) override;

            error apply_impl(std::string &) override;

            error apply_impl(std::u16string &) override;

            error apply_impl(std::u32string &) override;

        private:
            bool range_check(size_t read_size) {
                return current_ + read_size <= end_;
            }

            const byte *current_;
            const byte *end_;
        };
    }    // namespace mtl
}    // namespace nil
