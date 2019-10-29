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

#include <nil/mtl/downstream.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/make_counted.hpp>
#include <nil/mtl/make_source_result.hpp>
#include <nil/mtl/outbound_path.hpp>
#include <nil/mtl/stream_source.hpp>
#include <nil/mtl/stream_source_trait.hpp>

namespace nil {
    namespace mtl {
        namespace detail {

            template<class Driver>
            class stream_source_impl : public Driver::source_type {
            public:
                // -- member types -----------------------------------------------------------

                using super = typename Driver::source_type;

                using driver_type = Driver;

                // -- constructors, destructors, and assignment operators --------------------

                template<class... Ts>
                stream_source_impl(scheduled_actor *self, Ts &&... xs) :
                    stream_manager(self), super(self), driver_(std::forward<Ts>(xs)...), at_end_(false) {
                    // nop
                }

                // -- implementation of virtual functions ------------------------------------

                void shutdown() override {
                    super::shutdown();
                    at_end_ = true;
                }

                bool done() const override {
                    return this->pending_handshakes_ == 0 && at_end_ && this->out_.clean();
                }

                bool generate_messages() override {
                    MTL_LOG_TRACE("");
                    if (at_end_)
                        return false;
                    auto hint = this->out_.capacity();
                    MTL_LOG_DEBUG(MTL_ARG(hint));
                    if (hint == 0)
                        return false;
                    downstream<typename Driver::output_type> ds {this->out_.buf()};
                    driver_.pull(ds, hint);
                    if (driver_.done())
                        at_end_ = true;
                    return hint != this->out_.capacity();
                }

            protected:
                void finalize(const error &reason) override {
                    driver_.finalize(reason);
                }

                Driver driver_;

            private:
                bool at_end_;
            };

            template<class Driver, class... Ts>
            typename Driver::source_ptr_type make_stream_source(scheduled_actor *self, Ts &&... xs) {
                using impl = stream_source_impl<Driver>;
                return make_counted<impl>(self, std::forward<Ts>(xs)...);
            }

        }    // namespace detail
    }        // namespace mtl
}    // namespace nil
