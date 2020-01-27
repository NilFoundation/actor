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

#include <nil/mtl/serialization/binary_deserializer.hpp>

#include <iomanip>
#include <sstream>
#include <type_traits>

#include <nil/mtl/detail/ieee_754.hpp>
#include <nil/mtl/detail/network_order.hpp>
#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/sec.hpp>

namespace nil {
    namespace mtl {

        namespace {

            using apply_result = binary_deserializer::result_type;

            template<class T>
            apply_result apply_int(binary_deserializer &source, T &x) {
                auto tmp = std::make_unsigned_t<T> {};
                if (auto err = source.apply(as_writable_bytes(make_span(&tmp, 1))))
                    return err;
                x = static_cast<T>(detail::from_network_order(tmp));
                return none;
            }

            template<class T>
            apply_result apply_float(binary_deserializer &source, T &x) {
                auto tmp = typename detail::ieee_754_trait<T>::packed_type {};
                if (auto err = apply_int(source, tmp))
                    return err;
                x = detail::unpack754(tmp);
                return none;
            }

            // Does not perform any range checks.
            template<class T>
            void unsafe_apply_int(binary_deserializer &source, T &x) {
                std::make_unsigned_t<T> tmp;
                memcpy(&tmp, source.current(), sizeof(tmp));
                source.skip(sizeof(tmp));
                x = static_cast<T>(detail::from_network_order(tmp));
            }

        }    // namespace

        binary_deserializer::binary_deserializer(actor_system &sys) noexcept : context_(sys.dummy_execution_unit()) {
            // nop
        }

        apply_result binary_deserializer::begin_object(uint16_t &nr, std::string &name) {
            name.clear();
            if (auto err = apply(nr))
                return err;
            if (nr == 0)
                if (auto err = apply(name))
                    return err;
            return none;
        }

        apply_result binary_deserializer::end_object() noexcept {
            return none;
        }

        apply_result binary_deserializer::begin_sequence(size_t &list_size) noexcept {
            // Use varbyte encoding to compress sequence size on the wire.
            uint32_t x = 0;
            int n = 0;
            uint8_t low7;
            do {
                if (auto err = apply(low7))
                    return err;
                x |= static_cast<uint32_t>((low7 & 0x7F)) << (7 * n);
                ++n;
            } while (low7 & 0x80);
            list_size = x;
            return none;
        }

        apply_result binary_deserializer::end_sequence() noexcept {
            return none;
        }

        void binary_deserializer::skip(size_t num_bytes) noexcept {
            MTL_ASSERT(num_bytes <= remaining());
            current_ += num_bytes;
        }

        void binary_deserializer::reset(span<const byte> bytes) noexcept {
            current_ = bytes.data();
            end_ = current_ + bytes.size();
        }

        apply_result binary_deserializer::apply(bool &x) noexcept {
            int8_t tmp = 0;
            if (auto err = apply(tmp))
                return err;
            x = tmp != 0;
            return none;
        }

        apply_result binary_deserializer::apply(int8_t &x) noexcept {
            if (range_check(1)) {
                x = static_cast<int8_t>(*current_++);
                return none;
            }
            return sec::end_of_stream;
        }

        apply_result binary_deserializer::apply(uint8_t &x) noexcept {
            if (range_check(1)) {
                x = static_cast<uint8_t>(*current_++);
                return none;
            }
            return sec::end_of_stream;
        }

        apply_result binary_deserializer::apply(int16_t &x) noexcept {
            return apply_int(*this, x);
        }

        apply_result binary_deserializer::apply(uint16_t &x) noexcept {
            return apply_int(*this, x);
        }

        apply_result binary_deserializer::apply(int32_t &x) noexcept {
            return apply_int(*this, x);
        }

        apply_result binary_deserializer::apply(uint32_t &x) noexcept {
            return apply_int(*this, x);
        }

        apply_result binary_deserializer::apply(int64_t &x) noexcept {
            return apply_int(*this, x);
        }

        apply_result binary_deserializer::apply(uint64_t &x) noexcept {
            return apply_int(*this, x);
        }

        apply_result binary_deserializer::apply(float &x) noexcept {
            return apply_float(*this, x);
        }

        apply_result binary_deserializer::apply(double &x) noexcept {
            return apply_float(*this, x);
        }

        apply_result binary_deserializer::apply(long double &x) {
            // TODO: Our IEEE-754 conversion currently does not work for long double. The
            //       standard does not guarantee a fixed representation for this type, but
            //       on X86 we can usually rely on 80-bit precision. For now, we fall back
            //       to string conversion.
            std::string tmp;
            if (auto err = apply(tmp))
                return err;
            std::istringstream iss {tmp};
            if (iss >> x)
                return none;
            return sec::invalid_argument;
        }

        apply_result binary_deserializer::apply(span<byte> x) noexcept {
            if (!range_check(x.size()))
                return sec::end_of_stream;
            memcpy(x.data(), current_, x.size());
            current_ += x.size();
            return none;
        }

        apply_result binary_deserializer::apply(std::string &x) {
            x.clear();
            size_t str_size = 0;
            if (auto err = begin_sequence(str_size))
                return err;
            if (!range_check(str_size))
                return sec::end_of_stream;
            x.assign(reinterpret_cast<const char *>(current_), str_size);
            current_ += str_size;
            return end_sequence();
        }

        apply_result binary_deserializer::apply(std::u16string &x) {
            x.clear();
            size_t str_size = 0;
            if (auto err = begin_sequence(str_size))
                return err;
            if (!range_check(str_size * sizeof(uint16_t)))
                return sec::end_of_stream;
            for (size_t i = 0; i < str_size; ++i) {
                uint16_t tmp;
                unsafe_apply_int(*this, tmp);
                x.push_back(static_cast<char16_t>(tmp));
            }
            return end_sequence();
        }

        apply_result binary_deserializer::apply(std::u32string &x) {
            x.clear();
            size_t str_size = 0;
            if (auto err = begin_sequence(str_size))
                return err;
            if (!range_check(str_size * sizeof(uint32_t)))
                return sec::end_of_stream;
            for (size_t i = 0; i < str_size; ++i) {
                // The standard does not guarantee that char32_t is exactly 32 bits.
                uint32_t tmp;
                unsafe_apply_int(*this, tmp);
                x.push_back(static_cast<char32_t>(tmp));
            }
            return end_sequence();
        }

        apply_result binary_deserializer::apply(std::vector<bool> &x) {
            x.clear();
            size_t len = 0;
            if (auto err = begin_sequence(len))
                return err;
            if (len == 0)
                return end_sequence();
            size_t blocks = len / 8;
            for (size_t block = 0; block < blocks; ++block) {
                uint8_t tmp = 0;
                if (auto err = apply(tmp))
                    return err;
                x.emplace_back((tmp & 0b1000'0000) != 0);
                x.emplace_back((tmp & 0b0100'0000) != 0);
                x.emplace_back((tmp & 0b0010'0000) != 0);
                x.emplace_back((tmp & 0b0001'0000) != 0);
                x.emplace_back((tmp & 0b0000'1000) != 0);
                x.emplace_back((tmp & 0b0000'0100) != 0);
                x.emplace_back((tmp & 0b0000'0010) != 0);
                x.emplace_back((tmp & 0b0000'0001) != 0);
            }
            auto trailing_block_size = len % 8;
            if (trailing_block_size > 0) {
                uint8_t tmp = 0;
                if (auto err = apply(tmp))
                    return err;
                switch (trailing_block_size) {
                    case 7:
                        x.emplace_back((tmp & 0b0100'0000) != 0);
                        [[fallthrough]];
                    case 6:
                        x.emplace_back((tmp & 0b0010'0000) != 0);
                        [[fallthrough]];
                    case 5:
                        x.emplace_back((tmp & 0b0001'0000) != 0);
                        [[fallthrough]];
                    case 4:
                        x.emplace_back((tmp & 0b0000'1000) != 0);
                        [[fallthrough]];
                    case 3:
                        x.emplace_back((tmp & 0b0000'0100) != 0);
                        [[fallthrough]];
                    case 2:
                        x.emplace_back((tmp & 0b0000'0010) != 0);
                        [[fallthrough]];
                    case 1:
                        x.emplace_back((tmp & 0b0000'0001) != 0);
                        [[fallthrough]];
                    default:
                        break;
                }
            }
            return end_sequence();
        }
    }    // namespace mtl
}    // namespace nil
