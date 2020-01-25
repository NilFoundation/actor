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

#include <limits>

#include <nil/mtl/actor_system_config.hpp>
#include <nil/mtl/serialization/binary_deserializer.hpp>
#include <nil/mtl/callback.hpp>
#include <nil/mtl/detail/worker_hub.hpp>
#include <nil/mtl/error.hpp>
#include <nil/mtl/io/basp/buffer_type.hpp>
#include <nil/mtl/io/basp/connection_state.hpp>
#include <nil/mtl/io/basp/header.hpp>
#include <nil/mtl/io/basp/message_queue.hpp>
#include <nil/mtl/io/basp/message_type.hpp>
#include <nil/mtl/io/basp/routing_table.hpp>
#include <nil/mtl/io/basp/worker.hpp>
#include <nil/mtl/io/middleman.hpp>
#include <nil/mtl/variant.hpp>

namespace nil {
    namespace mtl {
        namespace io {
            namespace basp {

                /// @addtogroup BASP

                /// Describes a protocol instance managing multiple connections.
                class instance {
                public:
                    /// Provides a callback-based interface for certain BASP events.
                    class callee {
                    public:
                        // -- member types ---------------------------------------------------------

                        using buffer_type = std::vector<char>;

                        // -- constructors, destructors, and assignment operators ------------------

                        explicit callee(actor_system &sys, proxy_registry::backend &backend);

                        virtual ~callee();

                        // -- pure virtual functions -----------------------------------------------

                        /// Called if a server handshake was received and
                        /// the connection to `nid` is established.
                        virtual void finalize_handshake(const node_id &nid, actor_id aid,
                                                        std::set<std::string> &sigs) = 0;

                        /// Called whenever a direct connection was closed or a
                        /// node became unrechable for other reasons *before*
                        /// this node gets erased from the routing table.
                        /// @warning The implementing class must not modify the
                        ///          routing table from this callback.
                        virtual void purge_state(const node_id &nid) = 0;

                        /// Called whenever a remote node created a proxy
                        /// for one of our local actors.
                        virtual void proxy_announced(const node_id &nid, actor_id aid) = 0;

                        /// Called whenever BASP learns the ID of a remote node
                        /// to which it does not have a direct connection.
                        virtual void learned_new_node_directly(const node_id &nid, bool was_known_indirectly) = 0;

                        /// Called whenever BASP learns the ID of a remote node
                        /// to which it does not have a direct connection.
                        virtual void learned_new_node_indirectly(const node_id &nid) = 0;

                        /// Called if a heartbeat was received from `nid`
                        virtual void handle_heartbeat() = 0;

                        /// Returns the current MTL scheduler context.
                        virtual execution_unit *current_execution_unit() = 0;

                        /// Returns the actor namespace associated to this BASP protocol instance.
                        proxy_registry &proxies() {
                            return namespace_;
                        }

                        /// Returns a reference to the sent buffer.
                        virtual buffer_type &get_buffer(connection_handle hdl) = 0;

                        /// Flushes the underlying write buffer of `hdl`.
                        virtual void flush(connection_handle hdl) = 0;

                        /// Returns a handle to the callee actor.
                        virtual strong_actor_ptr this_actor() = 0;

                    protected:
                        proxy_registry namespace_;
                    };

                    /// Describes a function object responsible for writing
                    /// the payload for a BASP message.
                    using payload_writer = callback<error_code<sec>(binary_serializer &)>;

                    /// Describes a callback function object for `remove_published_actor`.
                    using removed_published_actor = callback<error_code<sec>(const strong_actor_ptr &, uint16_t)>;

                    instance(abstract_broker *parent, callee &lstnr);

                    /// Handles received data and returns a config for receiving the
                    /// next data or `none` if an error occured.
                    connection_state handle(execution_unit *ctx, new_data_msg &dm, header &hdr, bool is_payload);

                    /// Sends heartbeat messages to all valid nodes those are directly connected.
                    void handle_heartbeat(execution_unit *ctx);

                    /// Returns a route to `target` or `none` on error.
                    optional<routing_table::route> lookup(const node_id &target);

                    /// Flushes the underlying buffer of `path`.
                    void flush(const routing_table::route &path);

                    /// Sends a BASP message and implicitly flushes the output buffer of `r`.
                    /// This function will update `hdr.payload_len` if a payload was written.
                    void write(execution_unit *ctx, const routing_table::route &r, header &hdr,
                               payload_writer *writer = nullptr);

                    /// Adds a new actor to the map of published actors.
                    void add_published_actor(uint16_t port,
                                             strong_actor_ptr published_actor,
                                             std::set<std::string>
                                                 published_interface);

                    /// Removes the actor currently assigned to `port`.
                    size_t remove_published_actor(uint16_t port, removed_published_actor *cb = nullptr);

                    /// Removes `whom` if it is still assigned to `port` or from all of its
                    /// current ports if `port == 0`.
                    size_t remove_published_actor(const actor_addr &whom, uint16_t port,
                                                  removed_published_actor *cb = nullptr);

                    /// Returns `true` if a path to destination existed, `false` otherwise.
                    bool dispatch(execution_unit *ctx, const strong_actor_ptr &sender,
                                  const std::vector<strong_actor_ptr> &forwarding_stack, const node_id &dest_node,
                                  uint64_t dest_actor, uint8_t flags, message_id mid, const message &msg);

                    /// Returns the actor namespace associated to this BASP protocol instance.
                    proxy_registry &proxies() {
                        return callee_.proxies();
                    }

                    /// Returns the routing table of this BASP instance.
                    routing_table &tbl() {
                        return tbl_;
                    }

                    /// Stores the address of a published actor along with its publicly
                    /// visible messaging interface.
                    using published_actor = std::pair<strong_actor_ptr, std::set<std::string>>;

                    /// Maps ports to addresses and interfaces of published actors.
                    using published_actor_map = std::unordered_map<uint16_t, published_actor>;

                    /// Returns the current mapping of ports to addresses
                    /// and interfaces of published actors.
                    const published_actor_map &published_actors() const {
                        return published_actors_;
                    }

                    /// Writes a header followed by its payload to `storage`.
                    static void write(execution_unit *ctx, buffer_type &buf, header &hdr, payload_writer *pw = nullptr);

                    /// Writes the server handshake containing the information of the
                    /// actor published at `port` to `buf`. If `port == none` or
                    /// if no actor is published at this port then a standard handshake is
                    /// written (e.g. used when establishing direct connections on-the-fly).
                    void write_server_handshake(execution_unit *ctx, buffer_type &out_buf, optional<uint16_t> port);

                    /// Writes the client handshake to `buf`.
                    void write_client_handshake(execution_unit *ctx, buffer_type &buf);

                    /// Writes an `announce_proxy` to `buf`.
                    void write_monitor_message(execution_unit *ctx, buffer_type &buf, const node_id &dest_node,
                                               actor_id aid);

                    /// Writes a `kill_proxy` to `buf`.
                    void write_down_message(execution_unit *ctx, buffer_type &buf, const node_id &dest_node,
                                            actor_id aid, const error &rsn);

                    /// Writes a `heartbeat` to `buf`.
                    void write_heartbeat(execution_unit *ctx, buffer_type &buf);

                    const node_id &this_node() const {
                        return this_node_;
                    }

                    detail::worker_hub<worker> &hub() {
                        return hub_;
                    }

                    message_queue &queue() {
                        return queue_;
                    }

                    actor_system &system() {
                        return callee_.proxies().system();
                    }

                    const actor_system_config &config() {
                        return system().config();
                    }

                    bool handle(execution_unit *ctx, connection_handle hdl, header &hdr, std::vector<char> *payload);

                private:
                    void forward(execution_unit *ctx, const node_id &dest_node, const header &hdr,
                                 std::vector<char> &payload);

                    routing_table tbl_;
                    published_actor_map published_actors_;
                    node_id this_node_;
                    callee &callee_;
                    message_queue queue_;
                    detail::worker_hub<worker> hub_;
                };

                /// @}

            }    // namespace basp
        }        // namespace io
    }            // namespace mtl
}    // namespace nil