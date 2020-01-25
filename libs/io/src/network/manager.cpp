//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/mtl/io/network/manager.hpp>

#include <nil/mtl/logger.hpp>

#include <nil/mtl/io/abstract_broker.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                manager::manager() : parent_(nullptr) {
                    // nop
                }

                manager::~manager() {
                    // nop
                }

                void manager::set_parent(abstract_broker *ptr) {
                    parent_ = ptr != nullptr ? ptr->ctrl() : nullptr;
                }

                abstract_broker *manager::parent() {
                    return parent_ ? static_cast<abstract_broker *>(parent_->get()) : nullptr;
                }

                void manager::detach(execution_unit *, bool invoke_disconnect_message) {
                    MTL_LOG_TRACE(MTL_ARG(invoke_disconnect_message));
                    // This function gets called from the multiplexer when an error occurs or
                    // from the broker when closing this manager. In both cases, we need to make
                    // sure this manager does not receive further socket events.
                    remove_from_loop();
                    // Disconnect from the broker if not already detached.
                    if (!detached()) {
                        MTL_LOG_DEBUG("disconnect servant from broker");
                        auto raw_ptr = parent();
                        // Keep a strong reference to our parent until we go out of scope.
                        strong_actor_ptr ptr;
                        ptr.swap(parent_);
                        detach_from(raw_ptr);
                        if (invoke_disconnect_message) {
                            auto mptr = make_mailbox_element(nullptr, make_message_id(), {}, detach_message());
                            switch (raw_ptr->consume(*mptr)) {
                                case im_success:
                                    raw_ptr->finalize();
                                    break;
                                case im_skipped:
                                    raw_ptr->push_to_cache(std::move(mptr));
                                    break;
                                case im_dropped:
                                    MTL_LOG_INFO("broker dropped disconnect message");
                                    break;
                            }
                        }
                    }
                }

                void manager::io_failure(execution_unit *ctx, operation op) {
                    MTL_LOG_TRACE(MTL_ARG(op));
                    MTL_IGNORE_UNUSED(op);
                    detach(ctx, true);
                }

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
