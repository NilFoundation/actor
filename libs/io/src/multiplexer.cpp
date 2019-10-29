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

#include <nil/mtl/io/network/multiplexer.hpp>
#include <nil/mtl/io/network/default_multiplexer.hpp>    // default singleton

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                multiplexer::multiplexer(actor_system *sys) : execution_unit(sys), tid_(std::this_thread::get_id()) {
                    // nop
                }

                multiplexer_ptr multiplexer::make(actor_system &sys) {
                    MTL_LOG_TRACE("");
                    return multiplexer_ptr {new default_multiplexer(&sys)};
                }

                multiplexer_backend *multiplexer::pimpl() {
                    return nullptr;
                }

                multiplexer::supervisor::~supervisor() {
                    // nop
                }

                resumable::subtype_t multiplexer::runnable::subtype() const {
                    return resumable::function_object;
                }

                void multiplexer::runnable::intrusive_ptr_add_ref_impl() {
                    intrusive_ptr_add_ref(this);
                }

                void multiplexer::runnable::intrusive_ptr_release_impl() {
                    intrusive_ptr_release(this);
                }

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
