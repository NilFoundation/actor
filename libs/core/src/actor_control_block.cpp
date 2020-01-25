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

#include <nil/mtl/actor_control_block.hpp>

#include <nil/mtl/to_string.hpp>

#include <nil/mtl/message.hpp>
#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/proxy_registry.hpp>
#include <nil/mtl/abstract_actor.hpp>
#include <nil/mtl/mailbox_element.hpp>

#include <nil/mtl/detail/disposer.hpp>

namespace nil {
    namespace mtl {

        actor_addr actor_control_block::address() {
            return {this, true};
        }

        void actor_control_block::enqueue(strong_actor_ptr sender, message_id mid, message content,
                                          execution_unit *host) {
            get()->enqueue(std::move(sender), mid, std::move(content), host);
        }

        void actor_control_block::enqueue(mailbox_element_ptr what, execution_unit *host) {
            get()->enqueue(std::move(what), host);
        }

        bool intrusive_ptr_upgrade_weak(actor_control_block *x) {
            auto count = x->strong_refs.load();
            while (count != 0)
                if (x->strong_refs.compare_exchange_weak(count, count + 1, std::memory_order_relaxed))
                    return true;
            return false;
        }

        void intrusive_ptr_release_weak(actor_control_block *x) {
            // destroy object if last weak pointer expires
            if (x->weak_refs == 1 || x->weak_refs.fetch_sub(1, std::memory_order_acq_rel) == 1)
                x->block_dtor(x);
        }

        void intrusive_ptr_release(actor_control_block *x) {
            // release implicit weak pointer if the last strong ref expires
            // and destroy the data block
            if (x->strong_refs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                x->data_dtor(x->get());
                intrusive_ptr_release_weak(x);
            }
        }

        bool operator==(const strong_actor_ptr &x, const abstract_actor *y) {
            return x.get() == actor_control_block::from(y);
        }

        bool operator==(const abstract_actor *x, const strong_actor_ptr &y) {
            return actor_control_block::from(x) == y.get();
        }

        error_code<sec> load_actor(strong_actor_ptr &storage, execution_unit *ctx, actor_id aid, const node_id &nid) {
            if (ctx == nullptr)
                return sec::no_context;
            auto &sys = ctx->system();
            if (sys.node() == nid) {
                storage = sys.registry().get(aid);
                MTL_LOG_DEBUG("fetch actor handle from local actor registry: " << (storage ? "found" : "not found"));
                return none;
            }
            auto prp = ctx->proxy_registry_ptr();
            if (prp == nullptr)
                return sec::no_proxy_registry;
            // deal with (proxies for) remote actors
            storage = prp->get_or_put(nid, aid);
            return none;
        }

        error_code<sec> save_actor(strong_actor_ptr &storage, execution_unit *ctx, actor_id aid, const node_id &nid) {
            if (ctx == nullptr)
                return sec::no_context;
            auto &sys = ctx->system();
            // register locally running actors to be able to deserialize them later
            if (nid == sys.node())
                sys.registry().put(aid, storage);
            return none;
        }

        namespace {

            void append_to_string_impl(std::string &x, const actor_control_block *y) {
                if (y != nullptr) {
                    x += std::to_string(y->aid);
                    x += '@';
                    append_to_string(x, y->nid);
                } else {
                    x += "0@invalid-node";
                }
            }

            std::string to_string_impl(const actor_control_block *x) {
                std::string result;
                append_to_string_impl(result, x);
                return result;
            }

        }    // namespace

        std::string to_string(const strong_actor_ptr &x) {
            return to_string_impl(x.get());
        }

        void append_to_string(std::string &x, const strong_actor_ptr &y) {
            return append_to_string_impl(x, y.get());
        }

        std::string to_string(const weak_actor_ptr &x) {
            return to_string_impl(x.get());
        }

        void append_to_string(std::string &x, const weak_actor_ptr &y) {
            return append_to_string_impl(x, y.get());
        }

    }    // namespace mtl
}    // namespace nil
