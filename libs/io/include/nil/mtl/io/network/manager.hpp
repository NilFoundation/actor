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

#pragma once

#include <nil/mtl/message.hpp>
#include <nil/mtl/ref_counted.hpp>
#include <nil/mtl/intrusive_ptr.hpp>

#include <nil/mtl/io/fwd.hpp>
#include <nil/mtl/io/network/operation.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace network {

                /// A manager configures an I/O device and provides callbacks
                /// for various I/O operations.
                class manager : public ref_counted {
                public:
                    manager();

                    ~manager() override;

                    /// Sets the parent for this manager.
                    /// @pre `parent() == nullptr`
                    void set_parent(abstract_broker *ptr);

                    /// Returns the parent broker of this manager.
                    abstract_broker *parent();

                    /// Returns `true` if this manager has a parent, `false` otherwise.
                    inline bool detached() const {
                        return !parent_;
                    }

                    /// Detach this manager from its parent and invoke `detach_message()``
                    /// if `invoke_detach_message == true`.
                    void detach(execution_unit *ctx, bool invoke_disconnect_message);

                    /// Causes the manager to gracefully close its connection.
                    virtual void graceful_shutdown() = 0;

                    /// Removes the I/O device to the event loop of the middleman.
                    virtual void remove_from_loop() = 0;

                    /// Adds the I/O device to the event loop of the middleman.
                    virtual void add_to_loop() = 0;

                    /// Detaches this manager from its parent in case of an error.
                    void io_failure(execution_unit *ctx, operation op);

                    /// Get the address of the underlying I/O device.
                    virtual std::string addr() const = 0;

                protected:
                    /// Creates a message signalizing a disconnect to the parent.
                    virtual message detach_message() = 0;

                    /// Detaches this manager from `ptr`.
                    virtual void detach_from(abstract_broker *ptr) = 0;

                    strong_actor_ptr parent_;
                };

            }    // namespace network
        }        // namespace io
    }            // namespace mtl
}    // namespace nil
