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

            struct invalid_accept_handle_t {
                constexpr invalid_accept_handle_t() {
                    // nop
                }
            };

            constexpr invalid_accept_handle_t invalid_accept_handle = invalid_accept_handle_t {};

            /// Generic handle type for managing incoming connections.
            class accept_handle : public handle<accept_handle, invalid_accept_handle_t> {
            public:
                friend class handle<accept_handle, invalid_accept_handle_t>;

                using super = handle<accept_handle, invalid_accept_handle_t>;

                constexpr accept_handle() {
                    // nop
                }

                constexpr accept_handle(const invalid_accept_handle_t &) {
                    // nop
                }

                template<class Inspector>
                friend typename Inspector::result_type inspect(Inspector &f, accept_handle &x) {
                    return f(meta::type_name("accept_handle"), x.id_);
                }

            private:
                inline accept_handle(int64_t handle_id) : super(handle_id) {
                    // nop
                }
            };

        }    // namespace io
    }        // namespace actor
}    // namespace nil

namespace std {

    template<>
    struct hash<nil::actor::io::accept_handle> {
        size_t operator()(const nil::actor::io::accept_handle &hdl) const {
            hash<int64_t> f;
            return f(hdl.id());
        }
    };

}    // namespace std
