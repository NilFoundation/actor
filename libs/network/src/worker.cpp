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

#include <nil/mtl/network/basp/worker.hpp>

#include <nil/mtl/spawner.hpp>
#include <nil/mtl/byte.hpp>
#include <nil/mtl/network/basp/message_queue.hpp>
#include <nil/mtl/proxy_registry.hpp>
#include <nil/mtl/scheduler/abstract_coordinator.hpp>

namespace nil {
    namespace mtl {
        namespace network {
            namespace basp {

                // -- constructors, destructors, and assignment operators ----------------------

                worker::worker(hub_type &hub, message_queue &queue, proxy_registry &proxies) :
                    hub_(&hub), queue_(&queue), proxies_(&proxies), system_(&proxies.system()) {
                    MTL_IGNORE_UNUSED(pad_);
                }

                worker::~worker() {
                    // nop
                }

                // -- management ---------------------------------------------------------------

                void worker::launch(const node_id &last_hop, const basp::header &hdr, span<const byte> payload) {
                    msg_id_ = queue_->new_id();
                    last_hop_ = last_hop;
                    memcpy(&hdr_, &hdr, sizeof(basp::header));
                    payload_.assign(payload.begin(), payload.end());
                    ref();
                    system_->scheduler().enqueue(this);
                }

                // -- implementation of resumable ----------------------------------------------

                resumable::resume_result worker::resume(execution_unit *ctx, size_t) {
                    ctx->proxy_registry_ptr(proxies_);
                    handle_remote_message(ctx);
                    hub_->push(this);
                    return resumable::awaiting_message;
                }

            }    // namespace basp
        }        // namespace network
    }            // namespace mtl
}    // namespace nil