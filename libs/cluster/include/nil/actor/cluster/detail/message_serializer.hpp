//---------------------------------------------------------------------------//
// Copyright (c) 2018-2021 Mikhail Komarov <nemo@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#pragma once

#include <vector>
#include <string>

#include <nil/actor/rpc/rpc.hh>
#include <nil/actor/rpc/rpc_types.hh>
#include <nil/actor/net/inet_address.hh>
#include <nil/actor/net/ip.hh>

namespace nil::actor {
    namespace cluster {

        struct serializer { };

        using rpc_proto = nil::actor::rpc::protocol<nil::actor::cluster::serializer>;

        template<typename T, typename Output>
        inline static void write_arithmetic_type(Output &out, T v) {
            static_assert(std::is_arithmetic<T>::value, "must be arithmetic type");
            return out.write(reinterpret_cast<const char *>(&v), sizeof(T));
        }

        template<typename T, typename Input>
        inline static T read_arithmetic_type(Input &in) {
            static_assert(std::is_arithmetic<T>::value, "must be arithmetic type");
            T v;
            in.read(reinterpret_cast<char *>(&v), sizeof(T));
            return v;
        }

        // SCALARS

        template<typename Output>
        inline static void write(serializer, Output &output, int32_t v) {
            return write_arithmetic_type(output, v);
        }

        template<typename Output>
        inline static void write(serializer, Output &output, uint32_t v) {
            return write_arithmetic_type(output, v);
        }

        template<typename Output>
        inline static void write(serializer, Output &output, int64_t v) {
            return write_arithmetic_type(output, v);
        }

        template<typename Output>
        inline static void write(serializer, Output &output, uint64_t v) {
            return write_arithmetic_type(output, v);
        }

        template<typename Output>
        inline static void write(serializer, Output &output, double v) {
            return write_arithmetic_type(output, v);
        }

        template<typename Output>
        inline static void write(serializer, Output &output, bool v) {
            return write_arithmetic_type(output, (uint8_t)v);
        }

        template<typename Input>
        inline static int32_t read(serializer, Input &input, nil::actor::rpc::type<int32_t>) {
            return read_arithmetic_type<int32_t>(input);
        }

        template<typename Input>
        inline static uint32_t read(serializer, Input &input, nil::actor::rpc::type<uint32_t>) {
            return read_arithmetic_type<uint32_t>(input);
        }

        template<typename Input>
        inline static uint64_t read(serializer, Input &input, nil::actor::rpc::type<uint64_t>) {
            return read_arithmetic_type<uint64_t>(input);
        }

        template<typename Input>
        inline static uint64_t read(serializer, Input &input, nil::actor::rpc::type<int64_t>) {
            return read_arithmetic_type<int64_t>(input);
        }

        template<typename Input>
        inline static double read(serializer, Input &input, nil::actor::rpc::type<double>) {
            return read_arithmetic_type<double>(input);
        }

        template<typename Input>
        inline static bool read(serializer, Input &input, nil::actor::rpc::type<bool>) {
            return (bool)read_arithmetic_type<uint8_t>(input);
        }

        // BUILT-IN TYPES

        template<typename Output>
        inline static void write(serializer, Output &out, const std::string &v) {
            write_arithmetic_type(out, uint32_t(v.size()));
            out.write(v.c_str(), v.size());
        }

        template<typename Input>
        inline static std::string read(serializer, Input &in, nil::actor::rpc::type<std::string>) {
            auto size = read_arithmetic_type<uint32_t>(in);
            std::string ret(size, '\0');
            in.read(ret.data(), size);
            return ret;
        }

        template<typename Output>
        inline void write(serializer, Output &out, const nil::actor::sstring &v) {
            write_arithmetic_type(out, uint32_t(v.size()));
            out.write(v.c_str(), v.size());
        }

        template<typename Input>
        inline nil::actor::sstring read(serializer, Input &in, nil::actor::rpc::type<nil::actor::sstring>) {
            auto size = read_arithmetic_type<uint32_t>(in);
            nil::actor::sstring ret(nil::actor::sstring::initialized_later(), size);
            in.read(ret.begin(), size);
            return ret;
        }

        template<typename Output>
        inline void write(serializer s, Output &out, const nil::actor::socket_address &v) {
            write_arithmetic_type(out, uint32_t(v.addr().as_ipv4_address().ip.raw));
            write_arithmetic_type(out, uint16_t(v.port()));
        }

        template<typename Input>
        inline nil::actor::socket_address read(serializer s, Input &in,
                                               nil::actor::rpc::type<nil::actor::socket_address>) {
            auto endpoint = read_arithmetic_type<uint32_t>(in);
            auto port = read_arithmetic_type<uint16_t>(in);
            return nil::actor::socket_address(endpoint, port);
        }

        template<typename Output, typename T>
        inline void write(serializer s, Output &out, const std::vector<T> &v) {
            write_arithmetic_type(out, uint32_t(v.size()));
            for (const auto &item : v) {
                write(s, out, item);
            }
        }

        template<typename Input, typename T>
        inline std::vector<T> read(serializer s, Input &in, nil::actor::rpc::type<std::vector<T>>) {
            auto size = read_arithmetic_type<uint32_t>(in);
            std::vector<T> ret;
            for (int j = 0; j < size; ++j) {
                ret.emplace_back(read(s, in, nil::actor::rpc::type<T> {}));
            }
            return ret;
        }

        template<typename Output, typename... Args>
        inline void write(serializer s, Output &out, const std::tuple<Args...> &v) {
            std::apply([&](const auto &...e) { ([&](auto const &element) { write(s, out, element); }(e), ...); }, v);
        }

        template<typename Input, typename... Args>
        inline std::tuple<Args...> read(serializer s, Input &in, nil::actor::rpc::type<std::tuple<Args...>>) {
            return std::make_tuple((read(s, in, nil::actor::rpc::type<Args> {}))...);
        }

        // CLASS USER-PROVIDED

        // Fallback on object member
        template<typename Output, typename T>
        inline void write(serializer s, Output &out, const T &v) {
            v.serialize(s, out);
        }

        // Fallback on object member
        template<typename Input, typename T>
        inline T read(serializer s, Input &in, nil::actor::rpc::type<T>) {
            return T::deserialize(s, in);
        }
    }    // namespace cluster
}    // namespace nil::actor
