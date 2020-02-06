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

#include <nil/mtl/actor_ostream.hpp>

#include <nil/mtl/send.hpp>
#include <nil/mtl/scoped_actor.hpp>
#include <nil/mtl/abstract_actor.hpp>
#include <nil/mtl/default_attachable.hpp>

#include <nil/mtl/scheduler/abstract_coordinator.hpp>

namespace nil {
    namespace mtl {

        actor_ostream::actor_ostream(local_actor *self) :
            self_(self->id()), printer_(self->home_system().scheduler().printer()) {
            init(self);
        }

        actor_ostream::actor_ostream(scoped_actor &self) :
            self_(self->id()), printer_(self->home_system().scheduler().printer()) {
            init(actor_cast<abstract_actor *>(self));
        }

        actor_ostream &actor_ostream::write(std::string arg) {
            printer_->enqueue(
                make_mailbox_element(nullptr, make_message_id(), {}, add_atom::value, self_, std::move(arg)), nullptr);
            return *this;
        }

        actor_ostream &actor_ostream::flush() {
            printer_->enqueue(make_mailbox_element(nullptr, make_message_id(), {}, flush_atom::value, self_), nullptr);
            return *this;
        }

        void actor_ostream::redirect(abstract_actor *self, std::string fn, int flags) {
            if (self == nullptr)
                return;
            auto pr = self->home_system().scheduler().printer();
            pr->enqueue(make_mailbox_element(nullptr, make_message_id(), {}, redirect_atom::value, self->id(),
                                             std::move(fn), flags),
                        nullptr);
        }

        void actor_ostream::redirect_all(spawner &sys, std::string fn, int flags) {
            auto pr = sys.scheduler().printer();
            pr->enqueue(
                make_mailbox_element(nullptr, make_message_id(), {}, redirect_atom::value, std::move(fn), flags),
                nullptr);
        }

        void actor_ostream::init(abstract_actor *self) {
            if (!self->getf(abstract_actor::has_used_aout_flag))
                self->setf(abstract_actor::has_used_aout_flag);
        }

        actor_ostream aout(local_actor *self) {
            return actor_ostream {self};
        }

        actor_ostream aout(scoped_actor &self) {
            return actor_ostream {self};
        }

    }    // namespace mtl
}    // namespace nil

namespace std {

    nil::mtl::actor_ostream &endl(nil::mtl::actor_ostream &o) {
        return o.write("\n");
    }

    nil::mtl::actor_ostream &flush(nil::mtl::actor_ostream &o) {
        return o.flush();
    }

}    // namespace std
