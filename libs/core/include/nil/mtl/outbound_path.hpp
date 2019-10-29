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

#include <deque>
#include <vector>
#include <cstdint>
#include <cstddef>

#include <nil/mtl/actor_control_block.hpp>
#include <nil/mtl/downstream_msg.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/stream_aborter.hpp>
#include <nil/mtl/stream_slot.hpp>
#include <nil/mtl/system_messages.hpp>

#include <nil/mtl/detail/type_traits.hpp>

#include <nil/mtl/meta/type_name.hpp>

namespace nil {
    namespace mtl {

        /// State for a single path to a sink of a `downstream_manager`.
        class outbound_path {
        public:
            // -- member types -----------------------------------------------------------

            /// Propagates graceful shutdowns.
            using regular_shutdown = downstream_msg::close;

            /// Propagates errors.
            using irregular_shutdown = downstream_msg::forced_close;

            /// Stores batches until receiving corresponding ACKs.
            using cache_type = std::deque<std::pair<int64_t, downstream_msg::batch>>;

            // -- constants --------------------------------------------------------------

            /// Stream aborter flag to monitor a path.
            static constexpr const auto aborter_type = stream_aborter::sink_aborter;

            // -- constructors, destructors, and assignment operators --------------------

            /// Constructs a pending path for given slot and handle.
            outbound_path(stream_slot sender_slot, strong_actor_ptr receiver_hdl);

            ~outbound_path();

            // -- downstream communication -----------------------------------------------

            /// Sends an `open_stream_msg` handshake.
            static void emit_open(local_actor *self, stream_slot slot, strong_actor_ptr to, message handshake_data,
                                  stream_priority prio);

            /// Sends a `downstream_msg::batch` on this path. Decrements `open_credit` by
            /// `xs_size` and increments `next_batch_id` by 1.
            void emit_batch(local_actor *self, int32_t xs_size, message xs);

            template<class Iterator>
            Iterator emit_batches_impl(local_actor *self, Iterator i, Iterator e, bool force_underfull) {
                MTL_LOG_TRACE(MTL_ARG(force_underfull));
                MTL_ASSERT(desired_batch_size > 0);
                using type = detail::decay_t<decltype(*i)>;
                // Ship full batches.
                while (std::distance(i, e) >= desired_batch_size) {
                    std::vector<type> tmp(std::make_move_iterator(i), std::make_move_iterator(i + desired_batch_size));
                    emit_batch(self, desired_batch_size, make_message(std::move(tmp)));
                    i += desired_batch_size;
                }
                // Ship underful batch only if `force_underful` is set.
                if (i != e && force_underfull) {
                    std::vector<type> tmp(std::make_move_iterator(i), std::make_move_iterator(e));
                    auto tmp_size = static_cast<int32_t>(tmp.size());
                    emit_batch(self, tmp_size, make_message(std::move(tmp)));
                    return e;
                }
                return i;
            }

            /// Calls `emit_batch` for each chunk in the cache, whereas each chunk is of
            /// size `desired_batch_size`. Does nothing for pending paths.
            template<class T>
            void emit_batches(local_actor *self, std::vector<T> &cache, bool force_underfull) {
                MTL_LOG_TRACE(MTL_ARG(slots) << MTL_ARG(open_credit) << MTL_ARG(cache) << MTL_ARG(force_underfull));
                if (pending())
                    return;
                MTL_ASSERT(open_credit >= 0);
                MTL_ASSERT(desired_batch_size > 0);
                MTL_ASSERT(cache.size() <= std::numeric_limits<int32_t>::max());
                auto first = cache.begin();
                auto last = first + std::min(open_credit, static_cast<int32_t>(cache.size()));
                if (first == last)
                    return;
                auto i = emit_batches_impl(self, first, last, force_underfull);
                if (i == cache.end()) {
                    cache.clear();
                } else if (i != first) {
                    cache.erase(first, i);
                }
            }

            /// Sends a `downstream_msg::close` on this path.
            void emit_regular_shutdown(local_actor *self);

            /// Sends a `downstream_msg::forced_close` on this path.
            void emit_irregular_shutdown(local_actor *self, error reason);

            /// Sends a `downstream_msg::forced_close`.
            static void emit_irregular_shutdown(local_actor *self, stream_slots slots, const strong_actor_ptr &hdl,
                                                error reason);

            // -- properties -------------------------------------------------------------

            /// Returns whether this path is pending, i.e., didn't receive an `ack_open`
            /// yet.
            bool pending() const noexcept {
                return slots.receiver == invalid_stream_slot;
            }

            /// Returns whether no pending ACKs exist.
            bool clean() const noexcept {
                return next_batch_id == next_ack_id;
            }

            void set_desired_batch_size(int32_t value) noexcept;

            // -- member variables -------------------------------------------------------

            /// Slot IDs for sender (self) and receiver (hdl).
            stream_slots slots;

            /// Handle to the sink.
            strong_actor_ptr hdl;

            /// Next expected batch ID.
            int64_t next_batch_id;

            /// Currently available credit on this path.
            int32_t open_credit;

            /// Ideal batch size. Configured by the sink.
            int32_t desired_batch_size;

            /// ID of the first unacknowledged batch. Note that MTL uses accumulative
            /// ACKs, i.e., receiving an ACK with a higher ID is not an error.
            int64_t next_ack_id;

            /// Stores the maximum capacity of the downstream actor.
            int32_t max_capacity;

            /// Stores whether an outbound path is marked for removal. The
            /// `downstream_manger` no longer sends new batches to a closing path, but
            /// buffered batches are still shipped. The owning `stream_manager` removes
            /// the path when receiving an `upstream_msg::ack_batch` and no pending
            /// batches for this path exist.
            bool closing;
        };

        /// @relates outbound_path
        template<class Inspector>
        typename Inspector::result_type inspect(Inspector &f, outbound_path &x) {
            return f(meta::type_name("outbound_path"), x.slots, x.hdl, x.next_batch_id, x.open_credit,
                     x.desired_batch_size, x.next_ack_id);
        }

    }    // namespace mtl
}    // namespace nil
