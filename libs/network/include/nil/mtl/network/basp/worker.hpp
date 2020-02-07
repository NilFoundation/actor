//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2018-2020 Nil Foundation AG
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

#include <nil/mtl/config.hpp>
#include <nil/mtl/detail/abstract_worker.hpp>
#include <nil/mtl/detail/worker_hub.hpp>
#include <nil/mtl/network/basp/header.hpp>
#include <nil/mtl/network/basp/message_queue.hpp>
#include <nil/mtl/network/basp/remote_message_handler.hpp>
#include <nil/mtl/network/fwd.hpp>
#include <nil/mtl/node_id.hpp>
#include <nil/mtl/resumable.hpp>

namespace nil {
    namespace mtl {
        namespace network {
            namespace basp {

                /// Deserializes payloads for BASP messages asynchronously.
                class worker : public detail::abstract_worker, public remote_message_handler<worker> {
                public:
                    // -- friends ----------------------------------------------------------------

                    friend remote_message_handler<worker>;

                    // -- member types -----------------------------------------------------------

                    using super = detail::abstract_worker;

                    using scheduler_type = scheduler::abstract_coordinator;

                    using buffer_type = std::vector<byte>;

                    using hub_type = detail::worker_hub<worker>;

                    // -- constructors, destructors, and assignment operators --------------------

                    /// Only the ::worker_hub has access to the construtor.
                    worker(hub_type &hub, message_queue &queue, proxy_registry &proxies);

                    ~worker() override;

                    // -- management -------------------------------------------------------------

                    void launch(const node_id &last_hop, const basp::header &hdr, span<const byte> payload);

                    // -- implementation of resumable --------------------------------------------

                    resume_result resume(execution_unit *ctx, size_t) override;

                private:
                    // -- constants and assertions -----------------------------------------------

                    /// Stores how many bytes the "first half" of this object requires.
                    static constexpr size_t pointer_members_size = sizeof(hub_type *) + sizeof(message_queue *) +
                                                                   sizeof(proxy_registry *) + sizeof(spawner *);

                    static_assert(MTL_CACHE_LINE_SIZE > pointer_members_size, "invalid cache line size");

                    // -- member variables -------------------------------------------------------

                    /// Points to our home hub.
                    hub_type *hub_;

                    /// Points to the queue for establishing strict ordering.
                    message_queue *queue_;

                    /// Points to our proxy registry / factory.
                    proxy_registry *proxies_;

                    /// Points to the parent system.
                    spawner *system_;

                    /// Prevents false sharing when writing to `next`.
                    char pad_[MTL_CACHE_LINE_SIZE - pointer_members_size];

                    /// ID for local ordering.
                    uint64_t msg_id_;

                    /// Identifies the node that sent us `hdr_` and `payload_`.
                    node_id last_hop_;

                    /// The header for the next message. Either a direct_message or a
                    /// routed_message.
                    header hdr_;

                    /// Contains whatever this worker deserializes next.
                    buffer_type payload_;
                };

            }    // namespace basp
        }        // namespace network
    }            // namespace mtl
}    // namespace nil
