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

#include <nil/mtl/detail/merged_tuple.hpp>

#include <nil/mtl/index_mapping.hpp>
#include <nil/mtl/system_messages.hpp>

#include <nil/mtl/detail/disposer.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            merged_tuple::cow_ptr merged_tuple::make(message x, message y) {
                data_type data {x.vals(), y.vals()};
                mapping_type mapping;
                auto s = x.size();
                for (size_t i = 0; i < s; ++i) {
                    if (x.match_element<index_mapping>(i))
                        mapping.emplace_back(1, x.get_as<index_mapping>(i).value - 1);
                    else
                        mapping.emplace_back(0, i);
                }
                return cow_ptr {make_counted<merged_tuple>(std::move(data), std::move(mapping))};
            }

            merged_tuple::merged_tuple(data_type xs, mapping_type ys) :
                data_(std::move(xs)), type_token_(0xFFFFFFFF), mapping_(std::move(ys)) {
                MTL_ASSERT(!data_.empty());
                MTL_ASSERT(!mapping_.empty());
                // calculate type token
                for (auto &p : mapping_) {
                    type_token_ <<= 6;
                    type_token_ |= data_[p.first]->type_nr(p.second);
                }
            }

            merged_tuple *merged_tuple::copy() const {
                return new merged_tuple(data_, mapping_);
            }

            void *merged_tuple::get_mutable(size_t pos) {
                MTL_ASSERT(pos < mapping_.size());
                auto &p = mapping_[pos];
                return data_[p.first].unshared().get_mutable(p.second);
            }

            error merged_tuple::load(size_t pos, deserializer &source) {
                MTL_ASSERT(pos < mapping_.size());
                auto &p = mapping_[pos];
                return data_[p.first].unshared().load(p.second, source);
            }

            size_t merged_tuple::size() const noexcept {
                return mapping_.size();
            }

            uint32_t merged_tuple::type_token() const noexcept {
                return type_token_;
            }

            rtti_pair merged_tuple::type(size_t pos) const noexcept {
                MTL_ASSERT(pos < mapping_.size());
                auto &p = mapping_[pos];
                return data_[p.first]->type(p.second);
            }

            const void *merged_tuple::get(size_t pos) const noexcept {
                MTL_ASSERT(pos < mapping_.size());
                auto &p = mapping_[pos];
                return data_[p.first]->get(p.second);
            }

            std::string merged_tuple::stringify(size_t pos) const {
                MTL_ASSERT(pos < mapping_.size());
                auto &p = mapping_[pos];
                return data_[p.first]->stringify(p.second);
            }

            type_erased_value_ptr merged_tuple::copy(size_t pos) const {
                MTL_ASSERT(pos < mapping_.size());
                auto &p = mapping_[pos];
                return data_[p.first]->copy(p.second);
            }

            error merged_tuple::save(size_t pos, serializer &sink) const {
                MTL_ASSERT(pos < mapping_.size());
                auto &p = mapping_[pos];
                return data_[p.first]->save(p.second, sink);
            }

            const merged_tuple::mapping_type &merged_tuple::mapping() const {
                return mapping_;
            }

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
