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

#include <boost/preprocessor/seq/for_each_i.hpp>

#include <nil/actor/cluster/detail/message_handler_registry.hpp>

#include <nil/actor/detail/message_identifier.hpp>

/// \exclude
#define ULTRAMARINE_MAKE_REMOTE_ENDPOINT(a, data, i, name) \
    ultramarine::cluster::detail::register_remote_endpoint<data, KeyType>(&data::name, internal::message::name());

/// \exclude
#define ULTRAMARINE_REMOTE_MAKE_VTABLE(name, seq)                             \
    static constexpr void export_vtable() {                                   \
        BOOST_PP_SEQ_FOR_EACH_I(ULTRAMARINE_MAKE_REMOTE_ENDPOINT, name, seq); \
    }                                                                         \
    static inline ultramarine::cluster::detail::static_init init =            \
        ultramarine::cluster::detail::static_init(&export_vtable);\

