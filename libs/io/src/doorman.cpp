//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/io/doorman.hpp>

#include <nil/actor/logger.hpp>

#include <nil/actor/io/abstract_broker.hpp>

namespace nil {
    namespace actor {
        namespace io {

            doorman::doorman(accept_handle acc_hdl) : doorman_base(acc_hdl) {
                // nop
            }

            doorman::~doorman() {
                // nop
            }

            message doorman::detach_message() {
                return make_message(acceptor_closed_msg {hdl()});
            }

            bool doorman::new_connection(execution_unit *ctx, connection_handle x) {
                msg().handle = x;
                return invoke_mailbox_element(ctx);
            }

        }    // namespace io
    }        // namespace actor
}    // namespace nil
