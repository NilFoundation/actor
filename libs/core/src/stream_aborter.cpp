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

#include <nil/mtl/stream_aborter.hpp>

#include <nil/mtl/actor.hpp>
#include <nil/mtl/actor_cast.hpp>
#include <nil/mtl/downstream_msg.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/message.hpp>
#include <nil/mtl/no_stages.hpp>
#include <nil/mtl/system_messages.hpp>
#include <nil/mtl/upstream_msg.hpp>

namespace nil {
    namespace mtl {

        stream_aborter::~stream_aborter() {
            // nop
        }

        void stream_aborter::actor_exited(const error &rsn, execution_unit *host) {
            MTL_ASSERT(observed_ != observer_);
            auto observer = actor_cast<strong_actor_ptr>(observer_);
            if (observer != nullptr) {
                stream_slots slots {0, slot_};
                mailbox_element_ptr ptr;
                if (mode_ == source_aborter) {
                    using msg_type = downstream_msg::forced_close;
                    ptr = make_mailbox_element(nullptr, make_message_id(), no_stages,
                                               nil::mtl::make<msg_type>(slots, observed_, rsn));
                } else {
                    using msg_type = upstream_msg::forced_drop;
                    ptr = make_mailbox_element(nullptr, make_message_id(), no_stages,
                                               nil::mtl::make<msg_type>(slots, observed_, rsn));
                }
                observer->enqueue(std::move(ptr), host);
            }
        }

        bool stream_aborter::matches(const attachable::token &what) {
            if (what.subtype != attachable::token::stream_aborter)
                return false;
            auto &ot = *reinterpret_cast<const token *>(what.ptr);
            return ot.observer == observer_ && ot.slot == slot_;
        }

        stream_aborter::stream_aborter(actor_addr &&observed, actor_addr &&observer, stream_slot slot, mode m) :
            observed_(std::move(observed)), observer_(std::move(observer)), slot_(slot), mode_(m) {
            // nop
        }

        void stream_aborter::add(strong_actor_ptr observed, actor_addr observer, stream_slot slot, mode m) {
            MTL_LOG_TRACE(MTL_ARG(observed) << MTL_ARG(observer) << MTL_ARG(slot));
            auto ptr = make_stream_aborter(observed->address(), std::move(observer), slot, m);
            observed->get()->attach(std::move(ptr));
        }

        void stream_aborter::del(strong_actor_ptr observed, const actor_addr &observer, stream_slot slot, mode m) {
            MTL_LOG_TRACE(MTL_ARG(observed) << MTL_ARG(observer) << MTL_ARG(slot));
            token tk {observer, slot, m};
            observed->get()->detach(tk);
        }

        attachable_ptr make_stream_aborter(actor_addr observed, actor_addr observer, stream_slot slot,
                                           stream_aborter::mode m) {
            return attachable_ptr {new stream_aborter(std::move(observed), std::move(observer), slot, m)};
        }

    }    // namespace mtl
}    // namespace nil
