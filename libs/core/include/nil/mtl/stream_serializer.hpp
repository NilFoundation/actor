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

#include <string>
#include <limits>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <streambuf>
#include <type_traits>

#include <nil/mtl/sec.hpp>
#include <nil/mtl/config.hpp>
#include <nil/mtl/streambuf.hpp>
#include <nil/mtl/serializer.hpp>

#include <nil/mtl/detail/ieee_754.hpp>
#include <nil/mtl/detail/network_order.hpp>

namespace nil {
    namespace mtl {

        /// Implements the serializer interface with a binary serialization protocol.
        template<class Streambuf>
        class stream_serializer : public serializer {
            using streambuf_type = typename std::remove_reference<Streambuf>::type;
            using char_type = typename streambuf_type::char_type;
            using streambuf_base = std::basic_streambuf<char_type>;
            static_assert(std::is_base_of<streambuf_base, streambuf_type>::value,
                          "Streambuf must inherit from std::streambuf");

        public:
            template<class... Ts>
            explicit stream_serializer(actor_system &sys, Ts &&... xs) :
                serializer(sys), streambuf_ {std::forward<Ts>(xs)...} {
            }

            template<class... Ts>
            explicit stream_serializer(execution_unit *ctx, Ts &&... xs) :
                serializer(ctx), streambuf_ {std::forward<Ts>(xs)...} {
            }

            template<class S,
                     class = typename std::enable_if<
                         std::is_same<typename std::remove_reference<S>::type,
                                      typename std::remove_reference<Streambuf>::type>::value>::type>
            explicit stream_serializer(S &&sb) : serializer(nullptr), streambuf_(std::forward<S>(sb)) {
            }

            error begin_object(uint16_t &typenr, std::string &name) override {
                return error::eval([&] { return apply(typenr); }, [&] { return typenr == 0 ? apply(name) : error {}; });
            }

            error end_object() override {
                return none;
            }

            error begin_sequence(size_t &list_size) override {
                // TODO: protect with `if constexpr (sizeof(size_t) > sizeof(uint32_t))`
                //       when switching to C++17
                MTL_ASSERT(list_size <= std::numeric_limits<uint32_t>::max());
                // Serialize a `size_t` always in 32-bit, to guarantee compatibility with
                // 32-bit nodes in the network.
                return varbyte_encode(static_cast<uint32_t>(list_size));
            }

            error end_sequence() override {
                return none;
            }

            error apply_raw(size_t num_bytes, void *data) override {
                auto ssize = static_cast<std::streamsize>(num_bytes);
                auto n = streambuf_.sputn(reinterpret_cast<char_type *>(data), ssize);
                if (n != ssize)
                    return sec::end_of_stream;
                return none;
            }

        protected:
            // Encode an unsigned integral type as variable-byte sequence.
            template<class T>
            error varbyte_encode(T x) {
                static_assert(std::is_unsigned<T>::value, "T must be an unsigned type");
                // For 64-bit values, the encoded representation cannot get larger than 10
                // bytes. A scratch space of 16 bytes suffices as upper bound.
                uint8_t buf[16];
                auto i = buf;
                while (x > 0x7f) {
                    *i++ = (static_cast<uint8_t>(x) & 0x7f) | 0x80;
                    x >>= 7;
                }
                *i++ = static_cast<uint8_t>(x) & 0x7f;
                auto res = streambuf_.sputn(reinterpret_cast<char_type *>(buf), static_cast<std::streamsize>(i - buf));
                if (res != (i - buf))
                    return sec::end_of_stream;
                return none;
            }

            error apply_impl(int8_t &x) override {
                return apply_raw(sizeof(int8_t), &x);
            }

            error apply_impl(uint8_t &x) override {
                return apply_raw(sizeof(uint8_t), &x);
            }

            error apply_impl(int16_t &x) override {
                return apply_int(x);
            }

            error apply_impl(uint16_t &x) override {
                return apply_int(x);
            }

            error apply_impl(int32_t &x) override {
                return apply_int(x);
            }

            error apply_impl(uint32_t &x) override {
                return apply_int(x);
            }

            error apply_impl(int64_t &x) override {
                return apply_int(x);
            }

            error apply_impl(uint64_t &x) override {
                return apply_int(x);
            }

            error apply_impl(float &x) override {
                return apply_int(detail::pack754(x));
            }

            error apply_impl(double &x) override {
                return apply_int(detail::pack754(x));
            }

            error apply_impl(long double &x) override {
                // The IEEE-754 conversion does not work for long double
                // => fall back to string serialization (event though it sucks).
                std::ostringstream oss;
                oss << std::setprecision(std::numeric_limits<long double>::digits) << x;
                auto tmp = oss.str();
                return apply_impl(tmp);
            }

            error apply_impl(std::string &x) override {
                auto str_size = x.size();
                auto data = const_cast<char *>(x.c_str());
                return error::eval([&] { return begin_sequence(str_size); },
                                   [&] { return apply_raw(x.size(), data); },
                                   [&] { return end_sequence(); });
            }

            error apply_impl(std::u16string &x) override {
                auto str_size = x.size();
                return error::eval([&] { return begin_sequence(str_size); },
                                   [&] { return consume_range_c<uint16_t>(x); },
                                   [&] { return end_sequence(); });
            }

            error apply_impl(std::u32string &x) override {
                auto str_size = x.size();
                return error::eval([&] { return begin_sequence(str_size); },
                                   [&] { return consume_range_c<uint32_t>(x); },
                                   [&] { return end_sequence(); });
            }

            template<class T>
            error apply_int(T x) {
                using unsigned_type = typename std::make_unsigned<T>::type;
                auto y = detail::to_network_order(static_cast<unsigned_type>(x));
                return apply_raw(sizeof(T), &y);
            }

        private:
            Streambuf streambuf_;
        };

    }    // namespace mtl
}    // namespace nil
