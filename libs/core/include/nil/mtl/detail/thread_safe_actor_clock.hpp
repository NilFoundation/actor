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

#include <array>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include <nil/mtl/abstract_actor.hpp>
#include <nil/mtl/detail/ringbuffer.hpp>
#include <nil/mtl/detail/simple_actor_clock.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            class thread_safe_actor_clock : public simple_actor_clock {
            public:
                // -- constants --------------------------------------------------------------

                static constexpr size_t buffer_size = 64;

                // -- member types -----------------------------------------------------------

                using super = simple_actor_clock;

                // -- member functions -------------------------------------------------------

                void set_ordinary_timeout(time_point t, abstract_actor *self, atom_value type, uint64_t id) override;

                void set_request_timeout(time_point t, abstract_actor *self, message_id id) override;

                void set_multi_timeout(time_point t, abstract_actor *self, atom_value type, uint64_t id) override;

                void cancel_ordinary_timeout(abstract_actor *self, atom_value type) override;

                void cancel_request_timeout(abstract_actor *self, message_id id) override;

                void cancel_timeouts(abstract_actor *self) override;

                void schedule_message(time_point t, strong_actor_ptr receiver, mailbox_element_ptr content) override;

                void schedule_message(time_point t, group target, strong_actor_ptr sender, message content) override;

                void cancel_all() override;

                void run_dispatch_loop();

                void cancel_dispatch_loop();

            private:
                void push(event *ptr);

                /// Receives timer events from other threads.
                detail::ringbuffer<unique_event_ptr, buffer_size> queue_;

                /// Locally caches events for processing.
                std::array<unique_event_ptr, buffer_size> events_;
            };

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
