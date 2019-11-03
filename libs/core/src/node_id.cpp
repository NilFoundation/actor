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

#include <nil/mtl/node_id.hpp>

#include <cstdio>
#include <cstring>
#include <iterator>
#include <sstream>

#include <nil/crypto3/codec/algorithm/encode.hpp>
#include <nil/crypto3/codec/hex.hpp>

#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/ripemd.hpp>

#include <nil/mtl/config.hpp>
#include <nil/mtl/serialization/deserializer.hpp>
#include <nil/mtl/detail/get_mac_addresses.hpp>
#include <nil/mtl/detail/get_process_id.hpp>
#include <nil/mtl/detail/get_root_uuid.hpp>
#include <nil/mtl/detail/parser/ascii_to_int.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/make_counted.hpp>
#include <nil/mtl/sec.hpp>
#include <nil/mtl/serializer.hpp>
#include <nil/mtl/string_algorithms.hpp>

using namespace nil::crypto3;

namespace nil {
    namespace mtl {

        node_id::data::~data() {
            // nop
        }

        node_id::default_data::default_data() : pid_(0) {
            memset(host_.data(), 0, host_.size());
        }

        node_id::default_data::default_data(uint32_t pid, const host_id_type &host) : pid_(pid), host_(host) {
            // nop
        }

        namespace {

            std::atomic<uint8_t> system_id;

        }    // namespace

        node_id node_id::default_data::local(const actor_system_config &) {
            MTL_LOG_TRACE("");
            auto ifs = detail::get_mac_addresses();
            std::vector<std::string> macs;
            macs.reserve(ifs.size());
            for (auto &i : ifs)
                macs.emplace_back(std::move(i.second));
            auto hd_serial_and_mac_addr = join(macs, "") + detail::get_root_uuid();
            host_id_type hid = hash::hash<hash::ripemd160>(hd_serial_and_mac_addr);
            // This hack enables multiple actor systems in a single process by overriding
            // the last byte in the node ID with the actor system "ID".
            hid.back() = system_id.fetch_add(1);
            return make_node_id(detail::get_process_id(), hid);
        }

        bool node_id::default_data::valid(const host_id_type &x) noexcept {
            auto is_zero = [](uint8_t x) { return x == 0; };
            return !std::all_of(x.begin(), x.end(), is_zero);
        }

        bool node_id::default_data::valid() const noexcept {
            return pid_ != 0 && valid(host_);
        }

        size_t node_id::default_data::hash_code() const noexcept {
            // XOR the first few bytes from the node ID and the process ID.
            auto x = static_cast<size_t>(pid_);
            auto y = *reinterpret_cast<const size_t *>(host_.data());
            return x ^ y;
        }

        atom_value node_id::default_data::implementation_id() const noexcept {
            return class_id;
        }

        int node_id::default_data::compare(const data &other) const noexcept {
            if (this == &other)
                return 0;
            auto other_id = other.implementation_id();
            if (class_id != other_id)
                return nil::mtl::compare(class_id, other_id);
            auto &x = static_cast<const default_data &>(other);
            if (pid_ != x.pid_)
                return pid_ < x.pid_ ? -1 : 1;
            return memcmp(host_.data(), x.host_.data(), host_.size());
        }

        void node_id::default_data::print(std::string &dst) const {
            if (!valid()) {
                dst += "invalid-node";
                return;
            }
            std::string host_hex = crypto3::encode<crypto3::codec::hex<>>(host_);
            dst += host_hex + '#' + std::to_string(pid_);
        }

        error node_id::default_data::serialize(serializer &sink) const {
            return sink(pid_, host_);
        }

        error node_id::default_data::deserialize(deserializer &source) {
            return source(pid_, host_);
        }

        node_id::uri_data::uri_data(uri value) : value_(std::move(value)) {
            // nop
        }

        bool node_id::uri_data::valid() const noexcept {
            return !value_.empty();
        }

        size_t node_id::uri_data::hash_code() const noexcept {
            std::hash<uri> f;
            return f(value_);
        }

        atom_value node_id::uri_data::implementation_id() const noexcept {
            return class_id;
        }

        int node_id::uri_data::compare(const data &other) const noexcept {
            if (this == &other)
                return 0;
            auto other_id = other.implementation_id();
            if (class_id != other_id)
                return nil::mtl::compare(class_id, other_id);
            return value_.compare(static_cast<const uri_data &>(other).value_);
        }

        void node_id::uri_data::print(std::string &dst) const {
            if (!valid()) {
                dst += "invalid-node";
                return;
            }
            dst += to_string(value_);
        }

        error node_id::uri_data::serialize(serializer &sink) const {
            return sink(value_);
        }

        error node_id::uri_data::deserialize(deserializer &source) {
            return source(value_);
        }

        node_id &node_id::operator=(const none_t &) {
            data_.reset();
            return *this;
        }

        node_id::node_id(intrusive_ptr<data> data) : data_(std::move(data)) {
            // nop
        }

        node_id::~node_id() {
            // nop
        }

        node_id::operator bool() const {
            return static_cast<bool>(data_);
        }

        int node_id::compare(const node_id &other) const noexcept {
            if (this == &other || data_ == other.data_)
                return 0;
            if (data_ == nullptr)
                return other.data_ == nullptr ? 0 : -1;
            return other.data_ == nullptr ? 1 : data_->compare(*other.data_);
        }

        void node_id::swap(node_id &x) {
            data_.swap(x.data_);
        }

        error node_id::serialize(serializer &sink) const {
            if (data_ && data_->valid()) {
                if (auto err = sink(data_->implementation_id()))
                    return err;
                return data_->serialize(sink);
            }
            return sink(atom(""));
        }

        error node_id::deserialize(deserializer &source) {
            atom_value impl;
            if (auto err = source(impl))
                return err;
            if (impl == atom("")) {
                data_.reset();
                return none;
            }
            if (impl == default_data::class_id) {
                if (data_ == nullptr || data_->implementation_id() != default_data::class_id)
                    data_ = make_counted<default_data>();
                return data_->deserialize(source);
            } else if (impl == uri_data::class_id) {
                if (data_ == nullptr || data_->implementation_id() != uri_data::class_id)
                    data_ = make_counted<uri_data>();
                return data_->deserialize(source);
            }
            return sec::unknown_type;
        }

        error inspect(serializer &sink, const node_id &x) {
            return x.serialize(sink);
        }

        error inspect(deserializer &source, node_id &x) {
            return x.deserialize(source);
        }

        void append_to_string(std::string &str, const node_id &x) {
            if (x != none)
                x->print(str);
            else
                str += "invalid-node";
        }

        std::string to_string(const node_id &x) {
            std::string result;
            append_to_string(result, x);
            return result;
        }

        node_id make_node_id(uri from) {
            auto ptr = make_counted<node_id::uri_data>(std::move(from));
            return node_id {std::move(ptr)};
        }

        node_id make_node_id(uint32_t process_id, const node_id::default_data::host_id_type &host_id) {
            auto ptr = make_counted<node_id::default_data>(process_id, host_id);
            return node_id {std::move(ptr)};
        }

        optional<node_id> make_node_id(uint32_t process_id, const std::string &host_hash) {
            using node_data = node_id::default_data;
            if (host_hash.size() != node_data::host_id_size * 2)
                return none;
            detail::parser::ascii_to_int<16, uint8_t> xvalue;
            node_data::host_id_type host_id;
            for (size_t i = 0; i < node_data::host_id_size; i += 2) {
                // Read two characters, each representing 4 bytes.
                if (!isxdigit(host_hash[i]) || !isxdigit(host_hash[i + 1]))
                    return none;
                host_id[i / 2] = (xvalue(host_hash[i]) << 4) | xvalue(host_hash[i + 1]);
            }
            if (!node_data::valid(host_id))
                return none;
            return make_node_id(process_id, host_id);
        }

    }    // namespace mtl
}    // namespace nil
