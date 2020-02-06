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

#include <thread>
#include <fstream>
#include <cstring>
#include <sstream>
#include <iostream>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

#include <nil/mtl/abstract_actor.hpp>
#include <nil/mtl/atom.hpp>
#include <nil/mtl/config.hpp>
#include <nil/mtl/deep_to_string.hpp>
#include <nil/mtl/detail/arg_wrapper.hpp>
#include <nil/mtl/detail/log_level.hpp>
#include <nil/mtl/detail/pretty_type_name.hpp>
#include <nil/mtl/detail/ringbuffer.hpp>
#include <nil/mtl/detail/scope_guard.hpp>
#include <nil/mtl/detail/shared_spinlock.hpp>
#include <nil/mtl/fwd.hpp>
#include <nil/mtl/intrusive/drr_queue.hpp>
#include <nil/mtl/intrusive/fifo_inbox.hpp>
#include <nil/mtl/intrusive/singly_linked.hpp>
#include <nil/mtl/ref_counted.hpp>
#include <nil/mtl/string_view.hpp>
#include <nil/mtl/timestamp.hpp>
#include <nil/mtl/type_nr.hpp>
#include <nil/mtl/unifyn.hpp>

/*
 * To enable logging, you have to define MTL_DEBUG. This enables
 * MTL_LOG_ERROR messages. To enable more debugging output, you can
 * define MTL_LOG_LEVEL to:
 * 1: + warning
 * 2: + info
 * 3: + debug
 * 4: + trace (prints for each logged method entry and exit message)
 *
 * Note: this logger emits log4j style output; logs are best viewed
 *       using a log4j viewer, e.g., http://code.google.com/p/otroslogviewer/
 *
 */
namespace nil {
    namespace mtl {

        /// Centrally logs events from all actors in an actor system. To enable
        /// logging in your application, you need to define `MTL_LOG_LEVEL`. Per
        /// default, the logger generates log4j compatible output.
        class logger : public ref_counted {
        public:
            // -- friends ----------------------------------------------------------------

            friend class spawner;

            // -- constants --------------------------------------------------------------

            /// Configures the size of the circular event queue.
            static constexpr size_t queue_size = 128;

            // -- member types -----------------------------------------------------------

            /// Combines various logging-related flags and parameters into a bitfield.
            struct config {
                /// Stores `max(file_verbosity, console_verbosity)`.
                unsigned verbosity : 4;

                /// Configures the verbosity for file output.
                unsigned file_verbosity : 4;

                /// Configures the verbosity for console output.
                unsigned console_verbosity : 4;

                /// Configures whether the logger immediately writes its output in the
                /// calling thread, bypassing its queue. Use this option only in
                /// single-threaded test environments.
                bool inline_output : 1;

                /// Configures whether the logger generates colored output.
                bool console_coloring : 1;

                config();
            };

            /// Encapsulates a single logging event.
            struct event {
                // -- constructors, destructors, and assignment operators ------------------

                event() = default;

                event(event &&) = default;

                event(const event &) = default;

                event &operator=(event &&) = default;

                event &operator=(const event &) = default;

                event(unsigned lvl, unsigned line, atom_value cat, string_view full_fun, string_view fun,
                      string_view fn, std::string msg, std::thread::id t, actor_id a, timestamp ts);

                // -- member variables -----------------------------------------------------

                /// Level/priority of the event.
                unsigned level;

                /// Current line in the file.
                unsigned line_number;

                /// Name of the category (component) logging the event.
                atom_value category_name;

                /// Name of the current function as reported by `__PRETTY_FUNCTION__`.
                string_view pretty_fun;

                /// Name of the current function as reported by `__func__`.
                string_view simple_fun;

                /// Name of the current file.
                string_view file_name;

                /// User-provided message.
                std::string message;

                /// Thread ID of the caller.
                std::thread::id tid;

                /// Actor ID of the caller.
                actor_id aid;

                /// Timestamp of the event.
                timestamp tstamp;
            };

            /// Internal representation of format string entites.
            enum field_type {
                invalid_field,
                category_field,
                class_name_field,
                date_field,
                file_field,
                line_field,
                message_field,
                method_field,
                newline_field,
                priority_field,
                runtime_field,
                thread_field,
                actor_field,
                percent_sign_field,
                plain_text_field
            };

            /// Represents a single format string field.
            struct field {
                field_type kind;
                std::string text;
            };

            /// Stores a parsed format string as list of fields.
            using line_format = std::vector<field>;

            /// Utility class for building user-defined log messages with `MTL_ARG`.
            class line_builder {
            public:
                line_builder();

                template<class T>
                typename std::enable_if<!std::is_pointer<T>::value, line_builder &>::type operator<<(const T &x) {
                    if (!str_.empty())
                        str_ += " ";
                    str_ += deep_to_string(x);
                    return *this;
                }

                line_builder &operator<<(const local_actor *self);

                line_builder &operator<<(const std::string &str);

                line_builder &operator<<(string_view str);

                line_builder &operator<<(const char *str);

                line_builder &operator<<(char x);

                std::string get() const;

            private:
                std::string str_;
            };

            // -- constructors, destructors, and assignment operators --------------------

            ~logger() override;

            // -- logging ----------------------------------------------------------------

            /// Writes an entry to the event-queue of the logger.
            /// @thread-safe
            void log(event &&x);

            // -- properties -------------------------------------------------------------

            /// Returns the ID of the actor currently associated to the calling thread.
            actor_id thread_local_aid();

            /// Associates an actor ID to the calling thread and returns the last value.
            actor_id thread_local_aid(actor_id aid);

            /// Returns whether the logger is configured to accept input for given
            /// component and log level.
            bool accepts(unsigned level, atom_value component_name);

            /// Returns the output format used for the log file.
            const line_format &file_format() const {
                return file_format_;
            }

            /// Returns the output format used for the console.
            const line_format &console_format() const {
                return console_format_;
            }

            unsigned verbosity() const noexcept {
                return cfg_.verbosity;
            }

            unsigned file_verbosity() const noexcept {
                return cfg_.file_verbosity;
            }

            unsigned console_verbosity() const noexcept {
                return cfg_.console_verbosity;
            }

            // -- static utility functions -----------------------------------------------

            /// Renders the prefix (namespace and class) of a fully qualified function.
            static void render_fun_prefix(std::ostream &out, const event &x);

            /// Renders the name of a fully qualified function.
            static void render_fun_name(std::ostream &out, const event &x);

            /// Renders the difference between `t0` and `tn` in milliseconds.
            static void render_time_diff(std::ostream &out, timestamp t0, timestamp tn);

            /// Renders the date of `x` in ISO 8601 format.
            static void render_date(std::ostream &out, timestamp x);

            /// Parses `format_str` into a format description vector.
            /// @warning The returned vector can have pointers into `format_str`.
            static line_format parse_format(const std::string &format_str);

            /// Skips path in `filename`.
            static string_view skip_path(string_view filename);

            // -- utility functions ------------------------------------------------------

            /// Renders `x` using the line format `lf` to `out`.
            void render(std::ostream &out, const line_format &lf, const event &x) const;

            /// Returns a string representation of the joined groups of `x` if `x` is an
            /// actor with the `subscriber` mixin.
            template<class T>
            static typename std::enable_if<std::is_base_of<mixin::subscriber_base, T>::value, std::string>::type
                joined_groups_of(const T &x) {
                return deep_to_string(x.joined_groups());
            }

            /// Returns a string representation of an empty list if `x` is not an actor
            /// with the `subscriber` mixin.
            template<class T>
            static typename std::enable_if<!std::is_base_of<mixin::subscriber_base, T>::value, const char *>::type
                joined_groups_of(const T &x) {
                MTL_IGNORE_UNUSED(x);
                return "[]";
            }

            // -- thread-local properties ------------------------------------------------

            /// Stores the actor system for the current thread.
            static void set_current_actor_system(spawner *);

            /// Returns the logger for the current thread or `nullptr` if none is
            /// registered.
            static logger *current_logger();

        private:
            // -- constructors, destructors, and assignment operators --------------------

            logger(spawner &sys);

            // -- initialization ---------------------------------------------------------

            void init(spawner_config &cfg);

            bool open_file();

            // -- event handling ---------------------------------------------------------

            void handle_event(const event &x);

            void handle_file_event(const event &x);

            void handle_console_event(const event &x);

            void log_first_line();

            void log_last_line();

            // -- thread management ------------------------------------------------------

            void run();

            void start();

            void stop();

            // -- member variables -------------------------------------------------------

            // Configures logger_verbosity and output generation.
            config cfg_;

            // Filters events by component name.
            std::vector<atom_value> component_blacklist;

            // References the parent system.
            spawner &system_;

            // Guards aids_.
            detail::shared_spinlock aids_lock_;

            // Maps thread IDs to actor IDs.
            std::unordered_map<std::thread::id, actor_id> aids_;

            // Identifies the thread that called `logger::start`.
            std::thread::id parent_thread_;

            // Timestamp of the first log event.
            timestamp t0_;

            // Format for generating file output.
            line_format file_format_;

            // Format for generating logger_console output.
            line_format console_format_;

            // Stream for file output.
            std::fstream file_;

            // Filled with log events by other threads.
            detail::ringbuffer<event, queue_size> queue_;

            // Stores the assembled name of the log file.
            std::string file_name_;

            // Executes `logger::run`.
            std::thread thread_;
        };

        /// @relates logger::field_type
        std::string to_string(logger::field_type x);

        /// @relates logger::field
        std::string to_string(const logger::field &x);

        /// @relates logger::field
        bool operator==(const logger::field &x, const logger::field &y);

    }    // namespace mtl
}    // namespace nil

// -- macro constants ----------------------------------------------------------

/// Expands to a no-op.
#define MTL_VOID_STMT static_cast<void>(0)

#ifndef MTL_LOG_COMPONENT
/// Name of the current component when logging.
#define MTL_LOG_COMPONENT "mtl"
#endif    // MTL_LOG_COMPONENT

// -- utility macros -----------------------------------------------------------

#ifdef MTL_MSVC
/// Expands to a string representation of the current funciton name that
/// includes the full function name and its signature.
#define MTL_PRETTY_FUN __FUNCSIG__
#else    // MTL_MSVC
/// Expands to a string representation of the current funciton name that
/// includes the full function name and its signature.
#define MTL_PRETTY_FUN __PRETTY_FUNCTION__
#endif    // MTL_MSVC

/// Concatenates `a` and `b` to a single preprocessor token.
#define MTL_CAT(a, b) a##b

#define MTL_LOG_MAKE_EVENT(aid, component, loglvl, message)                                                         \
    ::nil::mtl::logger::event(loglvl, __LINE__, nil::mtl::atom(component), MTL_PRETTY_FUN, __func__,                \
                              nil::mtl::logger::skip_path(__FILE__),                                                \
                              (::nil::mtl::logger::line_builder {} << message).get(), ::std::this_thread::get_id(), \
                              aid, ::nil::mtl::make_timestamp())

/// Expands to `argument = <argument>` in log output.
#define MTL_ARG(argument) nil::mtl::detail::make_arg_wrapper(#argument, argument)

/// Expands to `argname = <argval>` in log output.
#define MTL_ARG2(argname, argval) nil::mtl::detail::make_arg_wrapper(argname, argval)

/// Expands to `argname = [argval, last)` in log output.
#define MTL_ARG3(argname, first, last) nil::mtl::detail::make_arg_wrapper(argname, first, last)

// -- logging macros -----------------------------------------------------------

#define MTL_LOG_IMPL(component, loglvl, message)                                                                     \
    do {                                                                                                             \
        auto MTL_UNIFYN(mtl_logger) = nil::mtl::logger::current_logger();                                            \
        if (MTL_UNIFYN(mtl_logger) != nullptr && MTL_UNIFYN(mtl_logger)->accepts(loglvl, nil::mtl::atom(component))) \
            MTL_UNIFYN(mtl_logger)                                                                                   \
                ->log(MTL_LOG_MAKE_EVENT(MTL_UNIFYN(mtl_logger)->thread_local_aid(), component, loglvl, message));   \
    } while (false)

#define MTL_PUSH_AID(aarg)                                                         \
    auto MTL_UNIFYN(mtl_tmp_ptr) = nil::mtl::logger::current_logger();             \
    nil::mtl::actor_id MTL_UNIFYN(mtl_aid_tmp) = 0;                                \
    if (MTL_UNIFYN(mtl_tmp_ptr))                                                   \
        MTL_UNIFYN(mtl_aid_tmp) = MTL_UNIFYN(mtl_tmp_ptr)->thread_local_aid(aarg); \
    auto MTL_UNIFYN(aid_aid_tmp_guard) = nil::mtl::detail::make_scope_guard([=] {  \
        auto MTL_UNIFYN(mtl_tmp2_ptr) = nil::mtl::logger::current_logger();        \
        if (MTL_UNIFYN(mtl_tmp2_ptr))                                              \
            MTL_UNIFYN(mtl_tmp2_ptr)->thread_local_aid(MTL_UNIFYN(mtl_aid_tmp));   \
    })

#define MTL_PUSH_AID_FROM_PTR(some_ptr)      \
    auto MTL_UNIFYN(mtl_aid_ptr) = some_ptr; \
    MTL_PUSH_AID(MTL_UNIFYN(mtl_aid_ptr) ? MTL_UNIFYN(mtl_aid_ptr)->id() : 0)

#define MTL_SET_AID(aid_arg) \
    (nil::mtl::logger::current_logger() ? nil::mtl::logger::current_logger()->thread_local_aid(aid_arg) : 0)

#define MTL_SET_LOGGER_SYS(ptr) nil::mtl::logger::set_current_actor_system(ptr)

#if MTL_LOG_LEVEL < MTL_LOG_LEVEL_TRACE

#define MTL_LOG_TRACE(unused) MTL_VOID_STMT

#else    // MTL_LOG_LEVEL < MTL_LOG_LEVEL_TRACE

#define MTL_LOG_TRACE(entry_message)                                                \
    MTL_LOG_IMPL(MTL_LOG_COMPONENT, MTL_LOG_LEVEL_TRACE, "ENTRY" << entry_message); \
    auto MTL_UNIFYN(mtl_log_trace_guard_) =                                         \
        ::nil::mtl::detail::make_scope_guard([=] { MTL_LOG_IMPL(MTL_LOG_COMPONENT, MTL_LOG_LEVEL_TRACE, "EXIT"); })

#endif    // MTL_LOG_LEVEL < MTL_LOG_LEVEL_TRACE

#if MTL_LOG_LEVEL >= MTL_LOG_LEVEL_DEBUG
#define MTL_LOG_DEBUG(output) MTL_LOG_IMPL(MTL_LOG_COMPONENT, MTL_LOG_LEVEL_DEBUG, output)
#endif

#if MTL_LOG_LEVEL >= MTL_LOG_LEVEL_INFO
#define MTL_LOG_INFO(output) MTL_LOG_IMPL(MTL_LOG_COMPONENT, MTL_LOG_LEVEL_INFO, output)
#endif

#if MTL_LOG_LEVEL >= MTL_LOG_LEVEL_WARNING
#define MTL_LOG_WARNING(output) MTL_LOG_IMPL(MTL_LOG_COMPONENT, MTL_LOG_LEVEL_WARNING, output)
#endif

#define MTL_LOG_ERROR(output) MTL_LOG_IMPL(MTL_LOG_COMPONENT, MTL_LOG_LEVEL_ERROR, output)

#ifndef MTL_LOG_INFO
#define MTL_LOG_INFO(output) MTL_VOID_STMT
#define MTL_LOG_INFO_IF(cond, output) MTL_VOID_STMT
#else    // MTL_LOG_INFO
#define MTL_LOG_INFO_IF(cond, output)                                \
    if (cond)                                                        \
        MTL_LOG_IMPL(MTL_LOG_COMPONENT, MTL_LOG_LEVEL_INFO, output); \
    MTL_VOID_STMT
#endif    // MTL_LOG_INFO

#ifndef MTL_LOG_DEBUG
#define MTL_LOG_DEBUG(output) MTL_VOID_STMT
#define MTL_LOG_DEBUG_IF(cond, output) MTL_VOID_STMT
#else    // MTL_LOG_DEBUG
#define MTL_LOG_DEBUG_IF(cond, output)                                \
    if (cond)                                                         \
        MTL_LOG_IMPL(MTL_LOG_COMPONENT, MTL_LOG_LEVEL_DEBUG, output); \
    MTL_VOID_STMT
#endif    // MTL_LOG_DEBUG

#ifndef MTL_LOG_WARNING
#define MTL_LOG_WARNING(output) MTL_VOID_STMT
#define MTL_LOG_WARNING_IF(cond, output) MTL_VOID_STMT
#else    // MTL_LOG_WARNING
#define MTL_LOG_WARNING_IF(cond, output)                                \
    if (cond)                                                           \
        MTL_LOG_IMPL(MTL_LOG_COMPONENT, MTL_LOG_LEVEL_WARNING, output); \
    MTL_VOID_STMT
#endif    // MTL_LOG_WARNING

#ifndef MTL_LOG_ERROR
#define MTL_LOG_ERROR(output) MTL_VOID_STMT
#define MTL_LOG_ERROR_IF(cond, output) MTL_VOID_STMT
#else    // MTL_LOG_ERROR
#define MTL_LOG_ERROR_IF(cond, output)                                \
    if (cond)                                                         \
        MTL_LOG_IMPL(MTL_LOG_COMPONENT, MTL_LOG_LEVEL_ERROR, output); \
    MTL_VOID_STMT
#endif    // MTL_LOG_ERROR

// -- macros for logging CE-0001 events ----------------------------------------

/// The log component responsible for logging control flow events that are
/// crucial for understanding happens-before relations. See RFC SE-0001.
#define MTL_LOG_FLOW_COMPONENT "mtl_flow"

#if MTL_LOG_LEVEL >= MTL_LOG_LEVEL_DEBUG

#define MTL_LOG_SPAWN_EVENT(ref, ctor_data)                                                        \
    MTL_LOG_IMPL(MTL_LOG_FLOW_COMPONENT, MTL_LOG_LEVEL_DEBUG,                                      \
                 "SPAWN ; ID =" << ref.id() << "; NAME =" << ref.name()                            \
                                << "; TYPE =" << ::nil::mtl::detail::pretty_type_name(typeid(ref)) \
                                << "; ARGS =" << ctor_data.c_str() << "; NODE =" << ref.node()     \
                                << "; GROUPS =" << ::nil::mtl::logger::joined_groups_of(ref))

#define MTL_LOG_SEND_EVENT(ptr)                                                                                   \
    MTL_LOG_IMPL(MTL_LOG_FLOW_COMPONENT, MTL_LOG_LEVEL_DEBUG,                                                     \
                 "SEND ; TO =" << ::nil::mtl::deep_to_string(::nil::mtl::strong_actor_ptr {this->ctrl()}).c_str() \
                               << "; FROM =" << ::nil::mtl::deep_to_string(ptr->sender).c_str()                   \
                               << "; STAGES =" << ::nil::mtl::deep_to_string(ptr->stages).c_str()                 \
                               << "; CONTENT =" << ::nil::mtl::deep_to_string(ptr->content()).c_str())

#define MTL_LOG_RECEIVE_EVENT(ptr)                                                                     \
    MTL_LOG_IMPL(MTL_LOG_FLOW_COMPONENT, MTL_LOG_LEVEL_DEBUG,                                          \
                 "RECEIVE ; FROM =" << ::nil::mtl::deep_to_string(ptr->sender).c_str()                 \
                                    << "; STAGES =" << ::nil::mtl::deep_to_string(ptr->stages).c_str() \
                                    << "; CONTENT =" << ::nil::mtl::deep_to_string(ptr->content()).c_str())

#define MTL_LOG_REJECT_EVENT() MTL_LOG_IMPL(MTL_LOG_FLOW_COMPONENT, MTL_LOG_LEVEL_DEBUG, "REJECT")

#define MTL_LOG_ACCEPT_EVENT(unblocked) \
    MTL_LOG_IMPL(MTL_LOG_FLOW_COMPONENT, MTL_LOG_LEVEL_DEBUG, "ACCEPT ; UNBLOCKED =" << unblocked)

#define MTL_LOG_DROP_EVENT() MTL_LOG_IMPL(MTL_LOG_FLOW_COMPONENT, MTL_LOG_LEVEL_DEBUG, "DROP")

#define MTL_LOG_SKIP_EVENT() MTL_LOG_IMPL(MTL_LOG_FLOW_COMPONENT, MTL_LOG_LEVEL_DEBUG, "SKIP")

#define MTL_LOG_FINALIZE_EVENT() MTL_LOG_IMPL(MTL_LOG_FLOW_COMPONENT, MTL_LOG_LEVEL_DEBUG, "FINALIZE")

#define MTL_LOG_TERMINATE_EVENT(thisptr, rsn)                                                       \
    MTL_LOG_IMPL(MTL_LOG_FLOW_COMPONENT, MTL_LOG_LEVEL_DEBUG,                                       \
                 "TERMINATE ; ID =" << thisptr->id() << "; REASON =" << deep_to_string(rsn).c_str() \
                                    << "; NODE =" << thisptr->node())

#else    // MTL_LOG_LEVEL >= MTL_LOG_LEVEL_DEBUG

#define MTL_LOG_SPAWN_EVENT(ref, ctor_data) MTL_VOID_STMT

#define MTL_LOG_SEND_EVENT(ptr) MTL_VOID_STMT

#define MTL_LOG_RECEIVE_EVENT(ptr) MTL_VOID_STMT

#define MTL_LOG_REJECT_EVENT() MTL_VOID_STMT

#define MTL_LOG_ACCEPT_EVENT(unblocked) MTL_VOID_STMT

#define MTL_LOG_DROP_EVENT() MTL_VOID_STMT

#define MTL_LOG_SKIP_EVENT() MTL_VOID_STMT

#define MTL_LOG_FINALIZE_EVENT() MTL_VOID_STMT

#define MTL_LOG_TERMINATE_EVENT(thisptr, rsn) MTL_VOID_STMT

#endif    // MTL_LOG_LEVEL >= MTL_LOG_LEVEL_DEBUG

// -- macros for logging streaming-related events ------------------------------

/// The log component for logging streaming-related events that are crucial for
/// understanding handshaking, credit decisions, etc.
#define MTL_LOG_STREAM_COMPONENT "mtl_stream"

#if MTL_LOG_LEVEL >= MTL_LOG_LEVEL_DEBUG
#define MTL_STREAM_LOG_DEBUG(output) MTL_LOG_IMPL(MTL_LOG_STREAM_COMPONENT, MTL_LOG_LEVEL_DEBUG, output)
#define MTL_STREAM_LOG_DEBUG_IF(condition, output) \
    if (condition)                                 \
    MTL_LOG_IMPL(MTL_LOG_STREAM_COMPONENT, MTL_LOG_LEVEL_DEBUG, output)
#else
#define MTL_STREAM_LOG_DEBUG(unused) MTL_VOID_STMT
#define MTL_STREAM_LOG_DEBUG_IF(unused1, unused2) MTL_VOID_STMT
#endif
