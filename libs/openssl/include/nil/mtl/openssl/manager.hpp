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

#include <set>
#include <string>

#include <nil/mtl/spawner.hpp>

#include <nil/mtl/io/middleman_actor.hpp>

namespace nil {
    namespace mtl {
        namespace openssl {

            /// Stores OpenSSL context information and provides access to necessary
            /// credentials for establishing connections.
            class manager : public spawner::module {
            public:
                ~manager() override;

                void start() override;

                void stop() override;

                void init(spawner_config &) override;

                id_t id() const override;

                void *subtype_ptr() override;

                /// Returns an SSL-aware implementation of the middleman actor interface.
                inline const io::middleman_actor &actor_handle() const {
                    return manager_;
                }

                /// Returns the enclosing actor system.
                inline spawner &system() {
                    return system_;
                }

                /// Returns the system-wide configuration.
                inline const spawner_config &config() const {
                    return system_.config();
                }

                /// Returns true if configured to require certificate-based authentication
                /// of peers.
                bool authentication_enabled();

                /// Returns an OpenSSL manager using the default network backend.
                /// @warning Creating an OpenSSL manager will fail when using
                //           a custom implementation.
                /// @throws `logic_error` if the middleman is not loaded or is not using the
                ///         default network backend.
                static spawner::module *make(spawner &, detail::type_list<>);

            private:
                /// Private since instantiation is only allowed via `make`.
                manager(spawner &sys);

                /// Reference to the parent.
                spawner &system_;

                /// OpenSSL-aware connection manager.
                io::middleman_actor manager_;
            };

        }    // namespace openssl
    }        // namespace mtl
}