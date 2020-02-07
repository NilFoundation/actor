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

#include <nil/mtl/detail/dynamic_message_data.hpp>

#include <nil/mtl/error.hpp>
#include <nil/mtl/intrusive_cow_ptr.hpp>
#include <nil/mtl/make_counted.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            dynamic_message_data::dynamic_message_data() : type_token_(0xFFFFFFFF) {
                // nop
            }

            dynamic_message_data::dynamic_message_data(elements &&data) :
                elements_(std::move(data)), type_token_(0xFFFFFFFF) {
                for (auto &e : elements_)
                    add_to_type_token(e->type().first);
            }

            dynamic_message_data::dynamic_message_data(const dynamic_message_data &other) :
                detail::message_data(other), type_token_(0xFFFFFFFF) {
                for (auto &e : other.elements_) {
                    add_to_type_token(e->type().first);
                    elements_.emplace_back(e->copy());
                }
            }

            dynamic_message_data::~dynamic_message_data() {
                // nop
            }

            dynamic_message_data *dynamic_message_data::copy() const {
                return new dynamic_message_data(*this);
            }

            void *dynamic_message_data::get_mutable(size_t pos) {
                MTL_ASSERT(pos < size());
                return elements_[pos]->get_mutable();
            }

            error dynamic_message_data::load(size_t pos, deserializer &source) {
                MTL_ASSERT(pos < size());
                return elements_[pos]->load(source);
            }

            error_code<sec> dynamic_message_data::load(size_t pos, binary_deserializer &source) {
                MTL_ASSERT(pos < size());
                return elements_[pos]->load(source);
            }

            size_t dynamic_message_data::size() const noexcept {
                return elements_.size();
            }

            uint32_t dynamic_message_data::type_token() const noexcept {
                return type_token_;
            }

            auto dynamic_message_data::type(size_t pos) const noexcept -> rtti_pair {
                MTL_ASSERT(pos < size());
                return elements_[pos]->type();
            }

            const void *dynamic_message_data::get(size_t pos) const noexcept {
                MTL_ASSERT(pos < size());
                return elements_[pos]->get();
            }

            std::string dynamic_message_data::stringify(size_t pos) const {
                MTL_ASSERT(pos < size());
                return elements_[pos]->stringify();
            }

            type_erased_value_ptr dynamic_message_data::copy(size_t pos) const {
                MTL_ASSERT(pos < size());
                return elements_[pos]->copy();
            }

            error dynamic_message_data::save(size_t pos, serializer &sink) const {
                MTL_ASSERT(pos < size());
                return elements_[pos]->save(sink);
            }

            error_code<sec> dynamic_message_data::save(size_t pos, binary_serializer &sink) const {
                MTL_ASSERT(pos < size());
                return elements_[pos]->save(sink);
            }

            void dynamic_message_data::clear() {
                elements_.clear();
                type_token_ = 0xFFFFFFFF;
            }

            void dynamic_message_data::append(type_erased_value_ptr x) {
                add_to_type_token(x->type().first);
                elements_.emplace_back(std::move(x));
            }

            void dynamic_message_data::add_to_type_token(uint16_t typenr) {
                type_token_ = (type_token_ << 6) | typenr;
            }

            void intrusive_ptr_add_ref(const dynamic_message_data *ptr) {
                intrusive_ptr_add_ref(static_cast<const ref_counted *>(ptr));
            }

            void intrusive_ptr_release(const dynamic_message_data *ptr) {
                intrusive_ptr_release(static_cast<const ref_counted *>(ptr));
            }

            dynamic_message_data *intrusive_cow_ptr_unshare(dynamic_message_data *&ptr) {
                return default_intrusive_cow_ptr_unshare(ptr);
            }

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
