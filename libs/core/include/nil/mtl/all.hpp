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

#include <nil/mtl/config.hpp>

#include <nil/mtl/sec.hpp>
#include <nil/mtl/atom.hpp>
#include <nil/mtl/send.hpp>
#include <nil/mtl/skip.hpp>
#include <nil/mtl/unit.hpp>
#include <nil/mtl/term.hpp>
#include <nil/mtl/actor.hpp>
#include <nil/mtl/after.hpp>
#include <nil/mtl/error.hpp>
#include <nil/mtl/group.hpp>
#include <nil/mtl/extend.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/others.hpp>
#include <nil/mtl/result.hpp>
#include <nil/mtl/stream.hpp>
#include <nil/mtl/message.hpp>
#include <nil/mtl/node_id.hpp>
#include <nil/mtl/behavior.hpp>
#include <nil/mtl/defaults.hpp>
#include <nil/mtl/duration.hpp>
#include <nil/mtl/expected.hpp>
#include <nil/mtl/exec_main.hpp>
#include <nil/mtl/resumable.hpp>
#include <nil/mtl/streambuf.hpp>
#include <nil/mtl/to_string.hpp>
#include <nil/mtl/actor_addr.hpp>
#include <nil/mtl/actor_pool.hpp>
#include <nil/mtl/attachable.hpp>
#include <nil/mtl/message_id.hpp>
#include <nil/mtl/replies_to.hpp>
#include <nil/mtl/serializer.hpp>
#include <nil/mtl/actor_clock.hpp>
#include <nil/mtl/actor_proxy.hpp>
#include <nil/mtl/exit_reason.hpp>
#include <nil/mtl/local_actor.hpp>
#include <nil/mtl/raise_error.hpp>
#include <nil/mtl/ref_counted.hpp>
#include <nil/mtl/stream_slot.hpp>
#include <nil/mtl/thread_hook.hpp>
#include <nil/mtl/typed_actor.hpp>
#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/config_value.hpp>
#include <nil/mtl/serialization/deserializer.hpp>
#include <nil/mtl/scoped_actor.hpp>
#include <nil/mtl/upstream_msg.hpp>
#include <nil/mtl/actor_ostream.hpp>
#include <nil/mtl/function_view.hpp>
#include <nil/mtl/index_mapping.hpp>
#include <nil/mtl/spawn_options.hpp>
#include <nil/mtl/abstract_actor.hpp>
#include <nil/mtl/abstract_group.hpp>
#include <nil/mtl/blocking_actor.hpp>
#include <nil/mtl/deep_to_string.hpp>
#include <nil/mtl/execution_unit.hpp>
#include <nil/mtl/memory_managed.hpp>
#include <nil/mtl/stateful_actor.hpp>
#include <nil/mtl/typed_behavior.hpp>
#include <nil/mtl/proxy_registry.hpp>
#include <nil/mtl/downstream_msg.hpp>
#include <nil/mtl/behavior_policy.hpp>
#include <nil/mtl/message_builder.hpp>
#include <nil/mtl/message_handler.hpp>
#include <nil/mtl/response_handle.hpp>
#include <nil/mtl/system_messages.hpp>
#include <nil/mtl/abstract_channel.hpp>
#include <nil/mtl/may_have_timeout.hpp>
#include <nil/mtl/message_priority.hpp>
#include <nil/mtl/typed_actor_view.hpp>
#include <nil/mtl/serialization/binary_serializer.hpp>
#include <nil/mtl/composed_behavior.hpp>
#include <nil/mtl/event_based_actor.hpp>
#include <nil/mtl/primitive_variant.hpp>
#include <nil/mtl/serialization/stream_serializer.hpp>
#include <nil/mtl/timeout_definition.hpp>
#include <nil/mtl/actor_system_config.hpp>
#include <nil/mtl/serialization/binary_deserializer.hpp>
#include <nil/mtl/composable_behavior.hpp>
#include <nil/mtl/serialization/stream_deserializer.hpp>
#include <nil/mtl/typed_actor_pointer.hpp>
#include <nil/mtl/scoped_execution_unit.hpp>
#include <nil/mtl/typed_response_promise.hpp>
#include <nil/mtl/typed_event_based_actor.hpp>
#include <nil/mtl/fused_downstream_manager.hpp>
#include <nil/mtl/abstract_composable_behavior.hpp>

#include <nil/mtl/decorator/sequencer.hpp>

#include <nil/mtl/meta/type_name.hpp>
#include <nil/mtl/meta/annotation.hpp>
#include <nil/mtl/meta/save_callback.hpp>
#include <nil/mtl/meta/load_callback.hpp>
#include <nil/mtl/meta/omittable_if_empty.hpp>

#include <nil/mtl/scheduler/test_coordinator.hpp>
#include <nil/mtl/scheduler/abstract_coordinator.hpp>

///
/// @mainpage MTL
///
/// @section Intro Introduction
///
/// This library provides an implementation of the actor model for C++.
/// It uses a network transparent messaging system to ease development
/// of both concurrent and distributed software.
///
/// `MTL` uses a thread pool to schedule actors by default.
/// A scheduled actor should not call blocking functions.
/// Individual actors can be spawned (created) with a special flag to run in
/// an own thread if one needs to make use of blocking APIs.
///
/// Writing applications in `MTL` requires a minimum of gluecode and
/// each context <i>is</i> an actor. Scoped actors allow actor interaction
/// from the context of threads such as main.
///
/// @section GettingStarted Getting Started
///
/// To build `MTL,` you need `GCC >= 4.8 or <tt>Clang >= 3.2</tt>,
/// and `CMake`.
///
/// The usual build steps on Linux and macOS are:
///
///- `./configure
///- `make
///- `make install (as root, optionally)
///
/// Please run the unit tests as well to verify that `libmtl`
/// works properly.
///
///- `make test
///
/// Please submit a bug report that includes (a) your compiler version,
/// (b) your OS, and (c) the output of the unit tests if an error occurs:
/// https://github.com/nilfoundation/mtl/issues
///
/// Please read the <b>Manual</b> for an introduction to `MTL`.
/// It is available online at https://mtl.nil.foundation
///
/// @section IntroHelloWorld Hello World Example
///
/// @include hello_world.cpp
///
/// @section IntroMoreExamples More Examples
///
/// The {@link math_actor.cpp Math Actor Example} shows the usage
/// of {@link receive_loop} and {@link nil::mtl::arg_match arg_match}.
/// The {@link dining_philosophers.cpp Dining Philosophers Example}
/// introduces event-based actors covers various features of MTL.
///
/// @namespace nil
/// Root namespace of all the Nil Foundation projects
///
/// @namespace mtl
/// Root namespace of MTL.
///
/// @namespace nil::mtl::mixin
/// Contains mixin classes implementing several actor traits.
///
/// @namespace nil::mtl::exit_reason
/// Contains all predefined exit reasons.
///
/// @namespace nil::mtl::policy
/// Contains policies encapsulating characteristics or algorithms.
///
/// @namespace nil::mtl::io
/// Contains all IO-related classes and functions.
///
/// @namespace nil::mtl::io::network
/// Contains classes and functions used for network abstraction.
///
/// @namespace nil::mtl::io::basp
/// Contains all classes and functions for the Binary Actor Sytem Protocol.
///
/// @defgroup MessageHandling Message Handling
///
/// This is the beating heart of MTL, since actor programming is
/// a message oriented programming paradigm.
///
/// A message in MTL is a n-tuple of values (with size >= 1).
/// You can use almost every type in a messages as long as it is announced,
/// i.e., known by the type system of MTL.
///
/// @defgroup BlockingAPI Blocking API
///
/// Blocking functions to receive messages.
///
/// The blocking API of MTL is intended to be used for migrating
/// previously threaded applications. When writing new code, you should
/// consider the nonblocking API based on `become` and `unbecome` first.
///
/// @section Send Sending Messages
///
/// The function `send` can be used to send a message to an actor.
/// The first argument is the receiver of the message followed by any number
/// of values:
///
/// ~~
/// // spawn some actors
/// actor_system_config cfg;
/// actor_system system{cfg};
/// auto a1 = system.spawn(...);
/// auto a2 = system.spawn(...);
/// auto a3 = system.spawn(...);
///
/// // an actor executed in the current thread
/// scoped_actor self{system};
///
/// // define an atom for message annotation
/// using hello_atom = atom_constant<atom("hello")>;
/// using compute_atom = atom_constant<atom("compute")>;
/// using result_atom = atom_constant<atom("result")>;
///
/// // send a message to a1
/// self->send(a1, hello_atom::value, "hello a1!");
///
/// // send a message to a1, a2, and a3
/// auto msg = make_message(compute_atom::value, 1, 2, 3);
/// self->send(a1, msg);
/// self->send(a2, msg);
/// self->send(a3, msg);
/// ~~
///
/// @section Receive Receive messages
///
/// The function `receive` takes a `behavior` as argument. The behavior
/// is a list of { callback } rules where the callback argument types
/// define a pattern for matching messages.
///
/// ~~
/// {
///   [](hello_atom, const std::string& msg) {
///     cout << "received hello message: " << msg << endl;
///   },
///   [](compute_atom, int i0, int i1, int i2) {
///     // send our result back to the sender of this messages
///     return make_message(result_atom::value, i0 + i1 + i2);
///   }
/// }
/// ~~
///
/// Blocking actors such as the scoped actor can call their receive member
/// to handle incoming messages.
///
/// ~~
/// self->receive(
///  [](result_atom, int i) {
///    cout << "result is: " << i << endl;
///  }
/// );
/// ~~
///
/// Please read the manual for further details about pattern matching.
///
/// @section Atoms Atoms
///
/// Atoms are a nice way to add semantic informations to a message.
/// Assuming an actor wants to provide a "math sevice" for integers. It
/// could provide operations such as addition, subtraction, etc.
/// This operations all have two operands. Thus, the actor does not know
/// what operation the sender of a message wanted by receiving just two integers.
///
/// Example actor:
/// ~~
/// using plus_atom = atom_constant<atom("plus")>;
/// using minus_atom = atom_constant<atom("minus")>;
/// behavior math_actor() {
///   return {
///     [](plus_atom, int a, int b) {
///       return make_message(atom("result"), a + b);
///     },
///     [](minus_atom, int a, int b) {
///       return make_message(atom("result"), a - b);
///     }
///   };
/// }
/// ~~
///
/// @section ReceiveLoops Receive Loops
///
/// The previous examples used `receive` to create a behavior on-the-fly.
/// This is inefficient in a loop since the argument passed to receive
/// is created in each iteration again. It's possible to store the behavior
/// in a variable and pass that variable to receive. This fixes the issue
/// of re-creation each iteration but rips apart definition and usage.
///
/// There are three convenience functions implementing receive loops to
/// declare behavior where it belongs without unnecessary
/// copies: `receive_while,` `receive_for` and `do_receive`.
///
/// `receive_while` creates a functor evaluating a lambda expression.
/// The loop continues until the given lambda returns `false`. A simple example:
///
/// ~~
/// size_t received = 0;
/// receive_while([&] { return received < 10; }) (
///   [&](int) {
///     ++received;
///   }
/// );
/// // ...
/// ~~
///
/// `receive_for` is a simple ranged-based loop:
///
/// ~~
/// std::vector<int> results;
/// size_t i = 0;
/// receive_for(i, 10) (
///   [&](int value) {
///     results.push_back(value);
///   }
/// );
/// ~~
///
/// `do_receive` returns a functor providing the function `until` that
/// takes a lambda expression. The loop continues until the given lambda
/// returns true. Example:
///
/// ~~
/// size_t received = 0;
/// do_receive (
///   [&](int) {
///     ++received;
///   }
/// ).until([&] { return received >= 10; });
/// // ...
/// ~~
///
/// @section FutureSend Sending Delayed Messages
///
/// The function `delayed_send` provides a simple way to delay a message.
/// This is particularly useful for recurring events, e.g., periodical polling.
/// Usage example:
///
/// ~~
/// scoped_actor self{...};
///
/// self->delayed_send(self, std::chrono::seconds(1), poll_atom::value);
/// bool running = true;
/// self->receive_while([&](){ return running; }) (
///   // ...
///   [&](poll_atom) {
///     // ... poll something ...
///     // and do it again after 1sec
///     self->delayed_send(self, std::chrono::seconds(1), poll_atom::value);
///   }
/// );
/// ~~
///
/// See also the {@link dancing_kirby.cpp dancing kirby example}.
///
/// @defgroup ImplicitConversion Implicit Type Conversions
///
/// The message passing of `libmtl` prohibits pointers in messages because
/// it enforces network transparent messaging.
/// Unfortunately, string literals in `C++` have the type `const char*,
/// resp. `const char[]. Since `libmtl` is a user-friendly library,
/// it silently converts string literals and C-strings to `std::string` objects.
/// It also converts unicode literals to the corresponding STL container.
///
/// A few examples:
/// ~~
/// // sends an std::string containing "hello actor!" to itself
/// send(self, "hello actor!");
///
/// const char* cstring = "cstring";
/// // sends an std::string containing "cstring" to itself
/// send(self, cstring);
///
/// // sends an std::u16string containing the UTF16 string "hello unicode world!"
/// send(self, u"hello unicode world!");
///
/// // x has the type nil::mtl::tuple<std::string, std::string>
/// auto x = make_message("hello", "tuple");
/// ~~
///
/// @defgroup ActorCreation Creating Actors

// examples

/// A trivial example program.
/// @example hello_world.cpp

/// A simple example for a delayed_send based application.
/// @example dancing_kirby.cpp

/// An event-based "Dining Philosophers" implementation.
/// @example dining_philosophers.cpp
