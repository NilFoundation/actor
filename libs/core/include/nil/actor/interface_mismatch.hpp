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

#include <nil/actor/fwd.hpp>
#include <nil/actor/replies_to.hpp>

#include <nil/actor/detail/type_list.hpp>

namespace nil {
    namespace actor {

        namespace detail {

            // imi = interface_mismatch_implementation
            // Precondition: Pos == 0 && len(Xs) == len(Ys) && len(Zs) == 0
            // Iterate over Xs to find a match in Ys; Zs is used as temporary storage
            // to iterate over Ys. On a match, the element is removed from Xs and Ys and
            // Zs are prepended to Ys again for next iteration.
            // Evaluates to:
            // * len(Xs) if interface check succeeds
            // * Pos on a mismatch (incremented per iteration to reflect position in Xs)
            template<int Pos, class Xs, class Ys, class Zs>
            struct imi;

            // end of recursion: success (consumed both lists)
            template<int Pos>
            struct imi<Pos, type_list<>, type_list<>, type_list<>> {
                static constexpr int value = Pos;
                using xs = type_list<>;
                using ys = type_list<>;
            };

            // end of recursion: success (consumed both lists, except the timeout)
            template<int Pos, class X>
            struct imi<Pos, type_list<timeout_definition<X>>, type_list<>, type_list<>> {
                static constexpr int value = Pos + 1;    // count timeout def. as consumed
                using xs = type_list<>;
                using ys = type_list<>;
            };

            // end of recursion: failure (consumed all Xs but not all Ys)
            template<int Pos, class Yin, class Yout, class... Ys>
            struct imi<Pos, type_list<>, type_list<typed_mpi<Yin, Yout>, Ys...>, type_list<>> {
                static constexpr int value = -1;
                using xs = type_list<>;
                using ys = type_list<typed_mpi<Yin, Yout>, Ys...>;
            };

            // end of recursion: failure (consumed all Ys but not all Xs)
            template<int Pos, class Xin, class Xout, class... Xs>
            struct imi<Pos, type_list<typed_mpi<Xin, Xout>, Xs...>, type_list<>, type_list<>> {
                static constexpr int value = -2;
                using xs = type_list<typed_mpi<Xin, Xout>, Xs...>;
                using ys = type_list<>;
            };

            // end of recursion: failure (consumed all Ys except timeout but not all Xs)
            template<int Pos, class X, class Y, class... Ys>
            struct imi<Pos, type_list<timeout_definition<X>>, type_list<Y, Ys...>, type_list<>> {
                static constexpr int value = -2;
                using xs = type_list<>;
                using ys = type_list<Y, Ys...>;
            };

            // case #1: exact match
            template<int Pos, class In, class Out, class... Xs, class... Ys, class... Zs>
            struct imi<Pos, type_list<typed_mpi<In, Out>, Xs...>, type_list<typed_mpi<In, Out>, Ys...>,
                       type_list<Zs...>> : imi<Pos + 1, type_list<Xs...>, type_list<Zs..., Ys...>, type_list<>> {};

            // case #2: match with skip_t
            template<int Pos, class In, class... Xs, class Out, class... Ys, class... Zs>
            struct imi<Pos, type_list<typed_mpi<In, output_tuple<skip_t>>, Xs...>, type_list<typed_mpi<In, Out>, Ys...>,
                       type_list<Zs...>> : imi<Pos + 1, type_list<Xs...>, type_list<Zs..., Ys...>, type_list<>> {};

            // case #3: no match at position
            template<int Pos, class Xin, class Xout, class... Xs, class Yin, class Yout, class... Ys, class... Zs>
            struct imi<Pos, type_list<typed_mpi<Xin, Xout>, Xs...>, type_list<typed_mpi<Yin, Yout>, Ys...>,
                       type_list<Zs...>> : imi<Pos, type_list<typed_mpi<Xin, Xout>, Xs...>, type_list<Ys...>,
                                               type_list<Zs..., typed_mpi<Yin, Yout>>> {};

            // case #4: no match (error)
            template<int Pos, class X, class... Xs, class... Zs>
            struct imi<Pos, type_list<X, Xs...>, type_list<>, type_list<Zs...>> {
                static constexpr int value = Pos;
                using xs = type_list<X, Xs...>;
                using ys = type_list<Zs...>;
            };

        }    // namespace detail

        /// Scans two typed MPI lists for compatibility, returning the index of the
        /// first mismatch. Returns the number of elements on a match.
        /// @pre len(Found) == len(Expected)
        template<class Found, class Expected>
        using interface_mismatch_t = detail::imi<0, Found, Expected, detail::type_list<>>;

    }    // namespace actor
}    // namespace nil
