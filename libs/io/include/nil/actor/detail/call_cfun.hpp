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

#include <cstdio>
#include <cstdlib>

#include <nil/actor/error.hpp>
#include <nil/actor/io/network/native_socket.hpp>
#include <nil/actor/sec.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            /// Predicate for `ccall` meaning "expected result of f is 0".
            inline bool cc_zero(int value) {
                return value == 0;
            }

            /// Predicate for `ccall` meaning "expected result of f is 1".
            inline bool cc_one(int value) {
                return value == 1;
            }

            /// Predicate for `ccall` meaning "expected result of f is not -1".
            inline bool cc_not_minus1(int value) {
                return value != -1;
            }

            /// Predicate for `ccall` meaning "expected result of f is a valid socket".
            inline bool cc_valid_socket(nil::actor::io::network::native_socket fd) {
                return fd != nil::actor::io::network::invalid_native_socket;
            }

/// Calls a C functions and returns an error if `predicate(var)` returns false.
#define CALL_CFUN(var, predicate, fun_name, expr) \
    auto var = expr;                              \
    if (!predicate(var))                          \
    return make_error(sec::network_syscall_failed, fun_name, last_socket_error_as_string())

/// Calls a C functions and calls exit() if `predicate(var)` returns false.
#define CALL_CRITICAL_CFUN(var, predicate, funname, expr)                                               \
    auto var = expr;                                                                                    \
    if (!predicate(var)) {                                                                              \
        fprintf(stderr, "[FATAL] %s:%u: syscall failed: %s returned %s\n", __FILE__, __LINE__, funname, \
                last_socket_error_as_string().c_str());                                                 \
        abort();                                                                                        \
    }                                                                                                   \
    static_cast<void>(0)

        }    // namespace detail
    }        // namespace actor
}    // namespace nil
