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

#include <nil/mtl/detail/set_thread_name.hpp>

#include <nil/mtl/config.hpp>

#ifndef MTL_WINDOWS
#include <pthread.h>
#endif    // MTL_WINDOWS

#if defined(MTL_LINUX)
#include <sys/prctl.h>
#elif defined(MTL_BSD)
#include <pthread_np.h>
#endif    // defined(...)

#include <thread>
#include <type_traits>

namespace nil {
    namespace mtl {
        namespace detail {

            void set_thread_name(const char *name) {
                MTL_IGNORE_UNUSED(name);
#ifdef MTL_WINDOWS
                // nop
#else    // MTL_WINDOWS
                static_assert(std::is_same<std::thread::native_handle_type, pthread_t>::value,
                              "std::thread not based on pthread_t");
#if defined(MTL_MACOS)
                pthread_setname_np(name);
#elif defined(MTL_LINUX)
                prctl(PR_SET_NAME, name, 0, 0, 0);
#elif defined(MTL_BSD)
                pthread_set_name_np(pthread_self(), name);
#endif    // defined(...)
#endif    // MTL_WINDOWS
            }

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
