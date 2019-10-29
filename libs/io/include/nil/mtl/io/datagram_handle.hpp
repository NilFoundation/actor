//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <functional>

#include <nil/mtl/error.hpp>

#include <nil/mtl/io/handle.hpp>

#include <nil/mtl/meta/type_name.hpp>

namespace nil {
    namespace mtl {
        namespace io {

            struct invalid_datagram_handle_t {
                constexpr invalid_datagram_handle_t() {
                    // nop
                }
            };

            constexpr invalid_datagram_handle_t invalid_datagram_handle = invalid_datagram_handle_t {};

            /// Generic handle type for identifying datagram endpoints
            class datagram_handle : public handle<datagram_handle, invalid_datagram_handle_t> {
            public:
                friend class handle<datagram_handle, invalid_datagram_handle_t>;

                using super = handle<datagram_handle, invalid_datagram_handle_t>;

                constexpr datagram_handle() {
                    // nop
                }

                constexpr datagram_handle(const invalid_datagram_handle_t &) {
                    // nop
                }

                template<class Inspector>
                friend typename Inspector::result_type inspect(Inspector &f, datagram_handle &x) {
                    return f(meta::type_name("datagram_handle"), x.id_);
                }

            private:
                inline datagram_handle(int64_t handle_id) : super {handle_id} {
                    // nop
                }
            };

        }    // namespace io
    }        // namespace mtl
}    // namespace nil

namespace std {

    template<>
    struct hash<nil::mtl::io::datagram_handle> {
        size_t operator()(const nil::mtl::io::datagram_handle &hdl) const {
            return std::hash<int64_t> {}(hdl.id());
        }
    };

}    // namespace std
