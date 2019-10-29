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

#include <tuple>

#include <nil/mtl/fwd.hpp>
#include <nil/mtl/intrusive_ptr.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/outbound_path.hpp>
#include <nil/mtl/stream_manager.hpp>

#include <nil/mtl/detail/type_traits.hpp>

namespace nil {
    namespace mtl {

        template<class DownstreamManager>
        class stream_source : public virtual stream_manager {
        public:
            // -- member types -----------------------------------------------------------

            using output_type = typename DownstreamManager::output_type;

            // -- constructors, destructors, and assignment operators --------------------

            stream_source(scheduled_actor *self) : stream_manager(self), out_(this) {
                // nop
            }

            bool idle() const noexcept override {
                // A source is idle if it can't make any progress on its downstream or if
                // it's not producing new data despite having credit.
                auto some_credit = [](const outbound_path &x) { return x.open_credit > 0; };
                return out_.stalled() || (out_.buffered() == 0 && out_.all_paths(some_credit));
            }

            DownstreamManager &out() override {
                return out_;
            }

            /// Creates a new output path to the current sender.
            outbound_stream_slot<output_type> add_outbound_path() {
                MTL_LOG_TRACE("");
                return this->add_unchecked_outbound_path<output_type>();
            }

            /// Creates a new output path to the current sender with custom handshake.
            template<class... Ts>
            outbound_stream_slot<output_type, detail::strip_and_convert_t<Ts>...>
                add_outbound_path(std::tuple<Ts...> xs) {
                MTL_LOG_TRACE(MTL_ARG(xs));
                return this->add_unchecked_outbound_path<output_type>(std::move(xs));
            }

            /// Creates a new output path to the current sender.
            template<class Handle>
            outbound_stream_slot<output_type> add_outbound_path(const Handle &next) {
                MTL_LOG_TRACE(MTL_ARG(next));
                return this->add_unchecked_outbound_path<output_type>(next);
            }

            /// Creates a new output path to the current sender with custom handshake.
            template<class Handle, class... Ts>
            outbound_stream_slot<output_type, detail::strip_and_convert_t<Ts>...>
                add_outbound_path(const Handle &next, std::tuple<Ts...> xs) {
                MTL_LOG_TRACE(MTL_ARG(next) << MTL_ARG(xs));
                return this->add_unchecked_outbound_path<output_type>(next, std::move(xs));
            }

        protected:
            DownstreamManager out_;
        };

        template<class DownstreamManager>
        using stream_source_ptr = intrusive_ptr<stream_source<DownstreamManager>>;

    }    // namespace mtl
}    // namespace nil
