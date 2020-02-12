//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2019 Nil Foundation AG
// Copyright (c) 2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.
//---------------------------------------------------------------------------//

#include <nil/actor/detail/fnv_hash.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            namespace {

                template<size_t IntegerSize>
                struct hash_conf_helper;

                template<>
                struct hash_conf_helper<4> {
                    static constexpr size_t basis = 2166136261u;

                    static constexpr size_t prime = 16777619u;
                };

                template<>
                struct hash_conf_helper<8> {
                    static constexpr size_t basis = 14695981039346656037u;

                    static constexpr size_t prime = 1099511628211u;
                };

                struct hash_conf : hash_conf_helper<sizeof(size_t)> {};

            }    // namespace

            size_t fnv_hash(const unsigned char *first, const unsigned char *last) {
                return fnv_hash_append(hash_conf::basis, first, last);
            }

            size_t fnv_hash_append(size_t intermediate, const unsigned char *first, const unsigned char *last) {
                auto result = intermediate;
                for (; first != last; ++first) {
                    result *= hash_conf::prime;
                    result ^= *first;
                }
                return result;
            }

        }    // namespace detail
    }        // namespace actor
}    // namespace nil
