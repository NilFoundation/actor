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

#include <boost/smart_ptr/intrusive_ptr.hpp>

#include <nil/mtl/detail/append_hex.hpp>
#include <nil/mtl/detail/comparable.hpp>
#include <nil/mtl/detail/type_traits.hpp>
#include <nil/mtl/fwd.hpp>

namespace nil {
    namespace mtl {

        template<typename T>
        using intrusive_ptr = boost::intrusive_ptr<T>;

        template<typename T>
        struct has_weak_ptr_semantics {
            constexpr static const bool value = false;
        };

        template<typename T>
        struct has_weak_ptr_semantics<intrusive_ptr<T>> {
            constexpr static const bool value = false;
        };

        template<class T>
        std::string to_string(const intrusive_ptr<T> &x) {
            std::string result;
            auto v = reinterpret_cast<uintptr_t>(x.get());
            detail::append_hex(result, reinterpret_cast<uint8_t *>(&v), sizeof(v));
            return result;
        }

    }    // namespace mtl
}    // namespace nil
