//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <functional>

#include <nil/actor/error.hpp>

#include <nil/actor/io/handle.hpp>

#include <nil/actor/meta/type_name.hpp>

namespace nil {
    namespace actor {
        namespace io {

            struct invalid_connection_handle_t {
                constexpr invalid_connection_handle_t() {
                    // nop
                }
            };

            constexpr invalid_connection_handle_t invalid_connection_handle = invalid_connection_handle_t {};

            /// Generic handle type for identifying connections.
            class connection_handle : public handle<connection_handle, invalid_connection_handle_t> {
            public:
                friend class handle<connection_handle, invalid_connection_handle_t>;

                using super = handle<connection_handle, invalid_connection_handle_t>;

                constexpr connection_handle() {
                    // nop
                }

                constexpr connection_handle(const invalid_connection_handle_t &) {
                    // nop
                }

                template<class Inspector>
                friend typename Inspector::result_type inspect(Inspector &f, connection_handle &x) {
                    return f(meta::type_name("connection_handle"), x.id_);
                }

            private:
                inline connection_handle(int64_t handle_id) : super(handle_id) {
                    // nop
                }
            };

        }    // namespace io
    }        // namespace actor
}    // namespace nil

namespace std {

    template<>
    struct hash<nil::actor::io::connection_handle> {
        size_t operator()(const nil::actor::io::connection_handle &hdl) const {
            hash<int64_t> f;
            return f(hdl.id());
        }
    };

}    // namespace std
