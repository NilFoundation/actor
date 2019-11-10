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

#include <nil/mtl/message.hpp>

#include <iostream>
#include <utility>

#include <nil/mtl/serialization/serializer.hpp>
#include <nil/mtl/serialization/deserializer.hpp>

#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/message_builder.hpp>
#include <nil/mtl/message_handler.hpp>

#include <nil/mtl/detail/decorated_tuple.hpp>
#include <nil/mtl/detail/concatenated_tuple.hpp>
#include <nil/mtl/detail/dynamic_message_data.hpp>

namespace nil {
    namespace mtl {

        message::message(none_t) noexcept {
            // nop
        }

        message::message(message &&other) noexcept : vals_(std::move(other.vals_)) {
            // nop
        }

        message::message(data_ptr ptr) noexcept : vals_(std::move(ptr)) {
            // nop
        }

        message &message::operator=(message &&other) noexcept {
            vals_.swap(other.vals_);
            return *this;
        }

        message::~message() {
            // nop
        }

        // -- implementation of type_erased_tuple --------------------------------------

        void *message::get_mutable(size_t p) {
            MTL_ASSERT(vals_ != nullptr);
            return vals_.unshared().get_mutable(p);
        }

        error message::load(size_t pos, deserializer &source) {
            MTL_ASSERT(vals_ != nullptr);
            return vals_.unshared().load(pos, source);
        }

        size_t message::size() const noexcept {
            return vals_ != nullptr ? vals_->size() : 0;
        }

        uint32_t message::type_token() const noexcept {
            return vals_ != nullptr ? vals_->type_token() : 0xFFFFFFFF;
        }

        rtti_pair message::type(size_t pos) const noexcept {
            MTL_ASSERT(vals_ != nullptr);
            return vals_->type(pos);
        }

        const void *message::get(size_t pos) const noexcept {
            MTL_ASSERT(vals_ != nullptr);
            return vals_->get(pos);
        }

        std::string message::stringify(size_t pos) const {
            MTL_ASSERT(vals_ != nullptr);
            return vals_->stringify(pos);
        }

        type_erased_value_ptr message::copy(size_t pos) const {
            MTL_ASSERT(vals_ != nullptr);
            return vals_->copy(pos);
        }

        error message::save(size_t pos, serializer &sink) const {
            MTL_ASSERT(vals_ != nullptr);
            return vals_->save(pos, sink);
        }

        bool message::shared() const noexcept {
            return vals_ != nullptr ? vals_->shared() : false;
        }

        error message::load(deserializer &source) {
            if (source.context() == nullptr)
                return sec::no_context;
            uint16_t zero;
            std::string tname;
            error err;
            err = source.begin_object(zero, tname);
            if (err)
                return err;
            if (zero != 0)
                return sec::unknown_type;
            if (tname == "@<>") {
                vals_.reset();
                return none;
            }
            if (tname.compare(0, 4, "@<>+") != 0)
                return sec::unknown_type;
            // iterate over concatenated type names
            auto eos = tname.end();
            auto next = [&](std::string::iterator iter) { return std::find(iter, eos, '+'); };
            auto &types = source.context()->system().types();
            auto dmd = make_counted<detail::dynamic_message_data>();
            std::string tmp;
            std::string::iterator i = next(tname.begin());
            ++i;    // skip first '+' sign
            do {
                auto n = next(i);
                tmp.assign(i, n);
                auto ptr = types.make_value(tmp);
                if (!ptr)
                    return make_error(sec::unknown_type, tmp);
                err = ptr->load(source);
                if (err)
                    return err;
                dmd->append(std::move(ptr));
                if (n != eos)
                    i = n + 1;
                else
                    i = eos;
            } while (i != eos);
            err = source.end_object();
            if (err)
                return err;
            message result {detail::message_data::cow_ptr {std::move(dmd)}};
            swap(result);
            return none;
        }

        error message::save(serializer &sink, const type_erased_tuple &x) {
            if (sink.context() == nullptr)
                return sec::no_context;
            // build type name
            uint16_t zero = 0;
            std::string tname = "@<>";
            if (x.empty())
                return error::eval([&] { return sink.begin_object(zero, tname); }, [&] { return sink.end_object(); });
            auto &types = sink.context()->system().types();
            auto n = x.size();
            for (size_t i = 0; i < n; ++i) {
                auto rtti = x.type(i);
                const auto &portable_name = types.portable_name(rtti);
                if (portable_name == types.default_type_name()) {
                    std::cerr << "[ERROR]: cannot serialize message because a type was "
                                 "not added to the types list, typeid name: "
                              << (rtti.second != nullptr ? rtti.second->name() : "-not-available-") << std::endl;
                    return make_error(sec::unknown_type,
                                      rtti.second != nullptr ? rtti.second->name() : "-not-available-");
                }
                tname += '+';
                tname += portable_name;
            }
            auto save_loop = [&]() -> error {
                for (size_t i = 0; i < n; ++i) {
                    auto e = x.save(i, sink);
                    if (e)
                        return e;
                }
                return none;
            };
            return error::eval([&] { return sink.begin_object(zero, tname); },
                               [&] { return save_loop(); },
                               [&] { return sink.end_object(); });
        }

        error message::save(serializer &sink) const {
            return save(sink, *this);
        }

        // -- factories ----------------------------------------------------------------

        message message::copy(const type_erased_tuple &xs) {
            message_builder mb;
            for (size_t i = 0; i < xs.size(); ++i)
                mb.emplace(xs.copy(i));
            return mb.move_to_message();
        }
        // -- modifiers ----------------------------------------------------------------

        message &message::operator+=(const message &x) {
            auto tmp = *this + x;
            swap(tmp);
            return *this;
        }

        optional<message> message::apply(message_handler handler) {
            return handler(*this);
        }

        void message::swap(message &other) noexcept {
            vals_.swap(other.vals_);
        }

        void message::reset(raw_ptr new_ptr, bool add_ref) noexcept {
            vals_.reset(new_ptr, add_ref);
        }

        // -- observers ----------------------------------------------------------------

        message message::drop(size_t n) const {
            MTL_ASSERT(vals_ != nullptr);
            if (n == 0)
                return *this;
            if (n >= size())
                return message {};
            std::vector<size_t> mapping(size() - n);
            size_t i = n;
            std::generate(mapping.begin(), mapping.end(), [&] { return i++; });
            return message {detail::decorated_tuple::make(vals_, mapping)};
        }

        message message::drop_right(size_t n) const {
            MTL_ASSERT(vals_ != nullptr);
            if (n == 0) {
                return *this;
            }
            if (n >= size()) {
                return message {};
            }
            std::vector<size_t> mapping(size() - n);
            std::iota(mapping.begin(), mapping.end(), size_t {0});
            return message {detail::decorated_tuple::make(vals_, std::move(mapping))};
        }

        message message::slice(size_t pos, size_t n) const {
            auto s = size();
            if (pos >= s) {
                return message {};
            }
            std::vector<size_t> mapping(std::min(s - pos, n));
            std::iota(mapping.begin(), mapping.end(), pos);
            return message {detail::decorated_tuple::make(vals_, std::move(mapping))};
        }

        message message::extract(message_handler handler) const {
            return extract_impl(0, std::move(handler));
        }

        // -- private helpers ----------------------------------------------------------

        message message::extract_impl(size_t start, message_handler handler) const {
            auto s = size();
            for (size_t i = start; i < s; ++i) {
                for (size_t n = (s - i); n > 0; --n) {
                    auto next_slice = slice(i, n);
                    auto res = handler(next_slice);
                    if (res) {
                        std::vector<size_t> mapping(s);
                        std::iota(mapping.begin(), mapping.end(), size_t {0});
                        auto first = mapping.begin() + static_cast<ptrdiff_t>(i);
                        auto last = first + static_cast<ptrdiff_t>(n);
                        mapping.erase(first, last);
                        if (mapping.empty()) {
                            return message {};
                        }
                        message next {detail::decorated_tuple::make(vals_, std::move(mapping))};
                        return next.extract_impl(i, handler);
                    }
                }
            }
            return *this;
        }

        message message::concat_impl(std::initializer_list<data_ptr> xs) {
            auto not_nullptr = [](const data_ptr &ptr) { return ptr.get() != nullptr; };
            switch (std::count_if(xs.begin(), xs.end(), not_nullptr)) {
                case 0:
                    return message {};
                case 1:
                    return message {*std::find_if(xs.begin(), xs.end(), not_nullptr)};
                default:
                    return message {detail::concatenated_tuple::make(xs)};
            }
        }

        // -- related non-members ------------------------------------------------------

        error inspect(serializer &sink, message &msg) {
            return msg.save(sink);
        }

        error inspect(deserializer &source, message &msg) {
            return msg.load(source);
        }

        std::string to_string(const message &msg) {
            if (msg.empty())
                return "<empty-message>";
            std::string str = "(";
            str += msg.cvals()->stringify(0);
            for (size_t i = 1; i < msg.size(); ++i) {
                str += ", ";
                str += msg.cvals()->stringify(i);
            }
            str += ")";
            return str;
        }

    }    // namespace mtl
}    // namespace nil
