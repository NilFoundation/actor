//---------------------------------------------------------------------------//
// Copyright (c) 2011-2017 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#pragma once

#include <tuple>
#include <vector>

#include <nil/mtl/fwd.hpp>
#include <nil/mtl/make_message.hpp>
#include <nil/mtl/message.hpp>
#include <nil/mtl/stream_sink.hpp>

namespace nil {
    namespace mtl {

        /// Identifies an unbound sequence of messages.
        template<class Input>
        class stream_sink_driver {
        public:
            // -- member types -----------------------------------------------------------

            using input_type = Input;

            /// Implemented `stream_sink` interface.
            using sink_type = stream_sink<input_type>;

            /// Smart pointer to the interface type.
            using sink_ptr_type = intrusive_ptr<sink_type>;

            // -- constructors, destructors, and assignment operators --------------------

            virtual ~stream_sink_driver() {
                // nop
            }

            // -- virtual functions ------------------------------------------------------

            /// Called after closing the last inbound path.
            virtual void finalize(const error &) {
                // nop
            }

            // -- pure virtual functions -------------------------------------------------

            /// Processes a single batch.
            virtual void process(std::vector<input_type> &batch) = 0;

            /// Can mark the sink as congested, e.g., when writing into a buffer that
            /// fills up faster than it is drained.
            virtual bool congested() const noexcept {
                return false;
            }

            /// Acquires credit on an inbound path. The calculated credit to fill our
            /// queue fro two cycles is `desired`, but the driver is allowed to return
            /// any non-negative value.
            virtual int32_t acquire_credit(inbound_path *path, int32_t desired) {
                MTL_IGNORE_UNUSED(path);
                return desired;
            }
        };

    }    // namespace mtl
}    // namespace nil
