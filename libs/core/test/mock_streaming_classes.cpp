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

// This test simulates a complex multiplexing over multiple layers of WDRR
// scheduled queues. The goal is to reduce the complex mailbox management of
// ACTOR to its bare bones in order to test whether the multiplexing of stream
// traffic and asynchronous messages works as intended.
//
// The setup is a fixed WDRR queue with three nestes queues. The first nested
// queue stores asynchronous messages, the second one upstream messages, and
// the last queue is a dynamic WDRR queue storing downstream messages.

#define BOOST_TEST_MODULE mock_streaming_classes_test

#include <boost/test/unit_test.hpp>

#include <memory>

#include <nil/actor/variant.hpp>
#include <nil/actor/stream_slot.hpp>
#include <nil/actor/logger.hpp>

#include <nil/actor/intrusive/drr_queue.hpp>
#include <nil/actor/intrusive/singly_linked.hpp>
#include <nil/actor/intrusive/task_result.hpp>
#include <nil/actor/intrusive/wdrr_dynamic_multiplexed_queue.hpp>
#include <nil/actor/intrusive/wdrr_fixed_multiplexed_queue.hpp>

#include <nil/actor/detail/overload.hpp>

using namespace nil::actor;
using namespace nil::actor::intrusive;

namespace {

    // -- utility ------------------------------------------------------------------

    struct print_with_comma_t {
        bool first = true;

        template<class T>
        std::ostream &operator()(std::ostream &out, const T &x) {
            if (!first) {
                out << ", ";
            } else {
                first = false;
            }
            return out << deep_to_string(x);
        }
    };

    template<class T, class... Ts>
    std::string collapse_args(const T &x, const Ts &... xs) {
        std::ostringstream out;
        print_with_comma_t f;
        f(out, x);
        unit(f(out, xs)...);
        return out.str();
    }

#define TRACE(name, type, ...) \
    BOOST_TEST_MESSAGE(name << " received a " << #type << ": " << collapse_args(__VA_ARGS__));

    // -- forward declarations -----------------------------------------------------

    // Mimics an actor.
    struct entity;

    // Mimics a stream_manager.
    struct manager;

    // Mimics a message.
    struct msg;

    // Mimics an inbound_path.
    struct in;

    // Mimics an outbound_path.
    struct out;

    // Mimics stream_handshake_msg.
    struct handshake;

    // Mimics downstream_msg.
    struct dmsg;

    // Mimics upstream_msg.
    struct umsg;

    // -- message types ------------------------------------------------------------

    struct handshake {
        stream_slot sender_slot;
    };

    struct dmsg {
        struct batch {
            std::vector<int> xs;
        };
        struct close {
            // nop
        };
        stream_slots slots;
        variant<batch, close> content;
    };

    struct umsg {
        struct ack_handshake {
            int32_t credit;
        };
        struct ack_batch {
            int32_t credit;
        };
        struct drop {
            // nop
        };
        stream_slots slots;
        variant<ack_batch, ack_handshake, drop> content;
    };

    struct msg : intrusive::singly_linked<msg> {
        entity *sender;
        variant<handshake, umsg, dmsg> content;

        template<class T>
        msg(entity *from, T &&x) : sender(from), content(std::forward<T>(x)) {
            // nop
        }
    };

    // -- manager and path handlers ------------------------------------------------

    struct manager {
        entity *self;
        int x;
        int num_messages;

        int input_paths = 0;
        int output_paths = 0;

        manager(entity *parent, int num = 0) : self(parent), x(0), num_messages(num) {
            // nop
        }

        bool done() const {
            return (input_paths | output_paths) == 0;
        }

        void push(entity *to, stream_slots slots, int num);

        void operator()(entity *sender, stream_slots slots, in *path, dmsg::batch &x);
    };

    using manager_ptr = std::shared_ptr<manager>;

    struct in {
        manager_ptr mgr;

        in(manager_ptr ptr) : mgr(std::move(ptr)) {
            // nop
        }

        void operator()(msg &x);
    };

    struct out {};

    // -- policies and queues ------------------------------------------------------

    struct policy_base {
        using mapped_type = msg;

        using task_size_type = size_t;

        using deficit_type = size_t;

        using deleter_type = std::default_delete<mapped_type>;

        using unique_pointer = std::unique_ptr<msg, deleter_type>;
    };

    struct handshake_queue_policy : policy_base {
        task_size_type task_size(const msg &) {
            return 1;
        }
    };

    using handshake_queue = drr_queue<handshake_queue_policy>;

    struct umsg_queue_policy : policy_base {
        manager *mgr;

        umsg_queue_policy(manager *ptr) : mgr(ptr) {
            // nop
        }

        task_size_type task_size(const msg &) {
            return 1;
        }
    };

    using umsg_queue = drr_queue<umsg_queue_policy>;

    struct inner_dmsg_queue_policy : policy_base {
        using key_type = stream_slot;

        task_size_type task_size(const msg &x) {
            return visit(*this, get<dmsg>(x.content).content);
        }

        task_size_type operator()(const dmsg::batch &x) const {
            BOOST_REQUIRE_NE(x.xs.size(), 0u);
            return x.xs.size();
        }

        task_size_type operator()(const dmsg::close &) const {
            return 1;
        }

        inner_dmsg_queue_policy(std::unique_ptr<in> ptr) : handler(std::move(ptr)) {
            // nop
        }

        std::unique_ptr<in> handler;
    };

    using inner_dmsg_queue = drr_queue<inner_dmsg_queue_policy>;

    struct dmsg_queue_policy : policy_base {
        using key_type = stream_slot;

        using queue_map_type = std::map<stream_slot, inner_dmsg_queue>;

        key_type id_of(const msg &x) {
            return get<dmsg>(x.content).slots.receiver;
        }

        template<class Queue>
        static inline bool enabled(const Queue &) {
            return true;
        }

        template<class Queue>
        deficit_type quantum(const Queue &, deficit_type x) {
            return x;
        }
    };

    using dmsg_queue = wdrr_dynamic_multiplexed_queue<dmsg_queue_policy>;

    struct mbox_policy : policy_base {
        template<class Queue>
        deficit_type quantum(const Queue &, deficit_type x) {
            return x;
        }

        size_t id_of(const msg &x) {
            return x.content.index();
        }
    };

    using mbox_queue = wdrr_fixed_multiplexed_queue<mbox_policy, handshake_queue, umsg_queue, dmsg_queue>;

    // -- manager and entity -------------------------------------------------------

    template<class Target>
    struct dispatcher {
        Target &f;
        entity *sender;
        stream_slots slots;

        template<class T>
        void operator()(T &&x) {
            f(sender, slots, std::forward<T>(x));
        }
    };

    struct entity {
        mbox_queue mbox;
        const char *name;

        entity(const char *cstr_name) :
            mbox(mbox_policy {}, handshake_queue_policy {}, umsg_queue_policy {nullptr}, dmsg_queue_policy {}),
            name(cstr_name) {
            // nop
        }

        void start_streaming(entity &to, int num_messages) {
            BOOST_REQUIRE_NE(num_messages, 0);
            auto slot = next_slot++;
            BOOST_TEST_MESSAGE(name << " starts streaming to " << to.name << " on slot " << slot);
            to.enqueue<handshake>(this, slot);
            auto ptr = std::make_shared<manager>(this, num_messages);
            ptr->output_paths += 1;
            pending_managers_.emplace(slot, std::move(ptr));
        }

        template<class T, class... Ts>
        bool enqueue(entity *sender, Ts &&... xs) {
            return mbox.emplace_back(sender, T {std::forward<Ts>(xs)...});
        }

        void operator()(entity *sender, handshake &hs) {
            TRACE(name, handshake, ACTOR_ARG2("sender", sender->name));
            auto slot = next_slot++;
            // stream_slots id{slot, hs.sender_slot};
            stream_slots id {hs.sender_slot, slot};
            // Create required state.
            auto mgr = std::make_shared<manager>(this, 0);
            managers_.emplace(id, mgr);
            mgr->input_paths += 1;
            // Create a new queue in the mailbox for incoming traffic.
            get<2>(mbox.queues()).queues().emplace(slot, inner_dmsg_queue_policy {std::unique_ptr<in> {new in(mgr)}});
            // Acknowledge stream.
            sender->enqueue<umsg>(this, id.invert(), umsg::ack_handshake {10});
        }

        void operator()(entity *sender, stream_slots slots, umsg::ack_handshake &x) {
            TRACE(name, ack_handshake, ACTOR_ARG(slots), ACTOR_ARG2("sender", sender->name));
            // Get the manager for that stream.
            auto i = pending_managers_.find(slots.receiver);
            BOOST_REQUIRE(i != pending_managers_.end());
            // Create a new queue in the mailbox for incoming traffic.
            // Swap the sender/receiver perspective to generate the ID we are using.
            managers_.emplace(slots, i->second);
            i->second->push(sender, slots.invert(), x.credit);
            pending_managers_.erase(i);
        }

        void operator()(entity *sender, stream_slots input_slots, umsg::ack_batch &x) {
            TRACE(name, ack_batch, ACTOR_ARG(input_slots), ACTOR_ARG2("sender", sender->name));
            // Get the manager for that stream.
            auto slots = input_slots.invert();
            auto i = managers_.find(input_slots);
            BOOST_REQUIRE(i != managers_.end());
            i->second->push(sender, slots, x.credit);
            if (i->second->done()) {
                managers_.erase(i);
            }
        }

        void operator()(entity *sender, stream_slots slots, in *, dmsg::close &) {
            TRACE(name, close, ACTOR_ARG(slots), ACTOR_ARG2("sender", sender->name));
            auto i = managers_.find(slots);
            BOOST_REQUIRE(i != managers_.end());
            i->second->input_paths -= 1;
            get<2>(mbox.queues()).erase_later(slots.receiver);
            if (i->second->done()) {
                BOOST_TEST_MESSAGE(name << " cleans up path " << deep_to_string(slots));
                managers_.erase(i);
            }
        }

        stream_slot next_slot = 1;

        std::map<stream_slot, int> generators;

        std::map<stream_slot, std::vector<int>> caches;

        struct source {
            stream_slot slot;
            entity *ptr;
        };

        std::map<stream_slot, manager_ptr> pending_managers_;

        std::map<stream_slots, manager_ptr> managers_;
    };

    void manager::push(entity *to, stream_slots slots, int num) {
        BOOST_REQUIRE_NE(num, 0);
        std::vector<int> xs;
        if (x + num > num_messages) {
            num = num_messages - x;
        }
        if (num == 0) {
            BOOST_TEST_MESSAGE(self->name << " is done sending batches");
            to->enqueue<dmsg>(self, slots, dmsg::close {});
            output_paths -= 1;
            return;
        }
        BOOST_TEST_MESSAGE(self->name << " pushes " << num << " new items to " << to->name
                                      << " slots = " << deep_to_string(slots));
        for (int i = 0; i < num; ++i) {
            xs.emplace_back(x++);
        }
        BOOST_REQUIRE_NE(xs.size(), 0u);
        auto emplace_res = to->enqueue<dmsg>(self, slots, dmsg::batch {std::move(xs)});
        BOOST_CHECK_EQUAL(emplace_res, true);
    }

    void manager::operator()(entity *sender, stream_slots slots, in *, dmsg::batch &batch) {
        TRACE(self->name, batch, ACTOR_ARG(slots), ACTOR_ARG2("sender", sender->name), ACTOR_ARG(batch.xs));
        sender->enqueue<umsg>(self, slots.invert(), umsg::ack_batch {10});
    }

    struct msg_visitor {
        entity *self;
        using is_handshake = std::integral_constant<size_t, 0>;
        using is_umsg = std::integral_constant<size_t, 1>;
        using is_dmsg = std::integral_constant<size_t, 2>;
        using result_type = intrusive::task_result;

        result_type operator()(is_handshake, handshake_queue &, msg &x) {
            (*self)(x.sender, get<handshake>(x.content));
            return intrusive::task_result::resume;
        }

        result_type operator()(is_umsg, umsg_queue &, msg &x) {
            auto sender = x.sender;
            auto &um = get<umsg>(x.content);
            auto f = detail::make_overload([&](umsg::ack_handshake &y) { (*self)(sender, um.slots, y); },
                                           [&](umsg::ack_batch &y) { (*self)(sender, um.slots, y); },
                                           [&](umsg::drop &) {
                                               //(*self)(sender, um.slots, y);
                                           });
            visit(f, um.content);
            return intrusive::task_result::resume;
        }

        result_type operator()(is_dmsg, dmsg_queue &, stream_slot, inner_dmsg_queue &q, msg &x) {
            auto inptr = q.policy().handler.get();
            auto dm = get<dmsg>(x.content);
            auto f = detail::make_overload([&](dmsg::batch &y) { (*inptr->mgr)(x.sender, dm.slots, inptr, y); },
                                           [&](dmsg::close &y) { (*self)(x.sender, dm.slots, inptr, y); });
            visit(f, dm.content);
            return intrusive::task_result::resume;
        }
    };

    // -- fixture ------------------------------------------------------------------

    struct fixture {
        entity alice {"alice"};
        entity bob {"bob"};

        fixture() {
            /// Make sure to test whether the slot IDs are properly handled.
            alice.next_slot = 123;
            bob.next_slot = 321;
        }
    };

}    // namespace

// -- unit tests ---------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(mock_streaming_classes_tests, fixture)

BOOST_AUTO_TEST_CASE(depth_2_pipeline_test) {
    alice.start_streaming(bob, 30);
    msg_visitor f {&bob};
    msg_visitor g {&alice};
    while (!bob.mbox.empty() || !alice.mbox.empty()) {
        bob.mbox.new_round(1, f);
        alice.mbox.new_round(1, g);
    }
    // Check whether bob and alice cleaned up their state properly.
    BOOST_CHECK(get<2>(bob.mbox.queues()).queues().empty());
    BOOST_CHECK(get<2>(alice.mbox.queues()).queues().empty());
    BOOST_CHECK(bob.pending_managers_.empty());
    BOOST_CHECK(alice.pending_managers_.empty());
    BOOST_CHECK(bob.managers_.empty());
    BOOST_CHECK(alice.managers_.empty());
}

BOOST_AUTO_TEST_SUITE_END()
