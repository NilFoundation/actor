//---------------------------------------------------------------------------//
// Copyright (c) 2018-2021 Mikhail Komarov <nemo@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#pragma once

#include <any>
#include <variant>

#include <nil/actor/detail/directory.hpp>
#include <nil/actor/detail/actor_ref_impl.hpp>

namespace nil::actor {

    /// A movable and copyable reference to a virtual actor
    /// \unique_name nil::actor::actor_ref
    /// \tparam Actor The type of [nil::actor::actor]() to reference
    /// \exclude
    template<typename Actor, ActorKind = actor_kind<Actor>()>
    class actor_ref { };

    /// A movable and copyable reference to a virtual actor
    /// \tparam Actor The type of [nil::actor::actor]() to reference
    /// \requires Type `Actor` shall inherit from [nil::actor::actor]()
    template<typename Actor>
    class actor_ref<Actor, ActorKind::SingletonActor> {
        detail::actor_ref_variant<Actor> impl;

    public:
        using ActorType = Actor;

        explicit constexpr actor_ref(detail::ActorKey<Actor> key) :
            impl(detail::wrap_actor_ref_impl<Actor>(std::move(key))) {
        }

        constexpr actor_ref(actor_ref const &) = default;

        constexpr actor_ref(actor_ref &&) noexcept = default;

    private:
        /// Obtain the concrete [nil::actor::actor_ref]() implementation
        /// \param func A lambda to execute with the [nil::actor::actor_ref]() implementation
        /// \returns The value returned by the provided lambda, if any
        template<typename Func>
        inline constexpr auto visit(Func &&func) const noexcept {
            return std::visit(
                [func = std::forward<Func>(func)](auto const &impl) mutable {
                    using ret_type = std::result_of_t<Func(decltype(impl))>;
                    if constexpr (nil::actor::is_future<ret_type>::value) {
                        return nil::actor::do_with(std::move(decltype(impl)(impl)),
                                                   [func](auto const &impl) mutable { return func(impl); });
                    } else {
                        return func(impl);
                    }
                },
                impl);
        }

    public:
        /// Provides an intuitive function call-like API.
        /// The syntax `ref->msg(args...)` is equivalent to `ref.tell(actor::message::msg, args...)` but shorter.
        /// \returns Returns the remote actor's interface
        inline constexpr typename Actor::internal::template interface<actor_ref<Actor>> operator->() const {
            return typename Actor::internal::template interface<actor_ref<Actor>> {*this};
        }

        /// Enqueue a message to the [nil::actor::actor]() referenced by this [nil::actor::actor_ref]() instance
        /// \effects Creates the [nil::actor::actor]() if it doesn't exist
        /// \param message The message handler to enqueue
        /// \returns A future representing the eventually returned value by the actor, or a failed future
        template<typename Handler, typename... Args>
        constexpr auto inline tell(Handler message, Args &&...args) const {
            return [this, message, args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                return visit([message, &args](auto const &impl) mutable {
                    return std::apply(
                        [&impl, message](auto &&...args) mutable {
                            return impl.tell(message, std::forward<Args>(args)...);
                        },
                        std::move(args));
                });
            }();
        }

        template<typename Handler, typename PackedArgs>
        constexpr auto inline tell_packed(Handler message, PackedArgs &&args) const {
            return visit([message, args = std::forward<PackedArgs>(args)](auto const &impl) mutable {
                return impl.tell_packed(message, std::forward<PackedArgs>(args));
            });
        }
    };

    /// A movable and copyable reference to an [nil::actor::actor]()
    /// \tparam Actor The type of [nil::actor::actor]() to reference
    /// \requires Type `Actor` shall inherit from [nil::actor::actor]() and from attribute [nil::actor::local_actor]()
    template<typename Actor>
    class actor_ref<Actor, ActorKind::LocalActor> {
        detail::ActorKey<Actor> key;

    public:
        using ActorType = Actor;

        explicit constexpr actor_ref(detail::ActorKey<Actor> key) : key(std::move(key)) {
        }

        constexpr actor_ref(actor_ref const &) = default;

        constexpr actor_ref(actor_ref &&) noexcept = default;

    private:
        /// Obtain the concrete [nil::actor::actor_ref]() implementation
        /// \param func A lambda to execute with the [nil::actor::actor_ref]() implementation
        /// \returns The value returned by the provided lambda, if any
        template<typename Func>
        inline constexpr auto visit(Func &&func) const noexcept {
            nil::actor::shard_id next = 0;

            if constexpr (is_unlimited_concurrent_local_actor_v<Actor>) {
                next = (Actor::round_robin_counter++ + nil::actor::engine().cpu_id()) % nil::actor::smp::count;
            } else {
                next =
                    (Actor::round_robin_counter++ + nil::actor::engine().cpu_id()) %
                    (nil::actor::smp::count < Actor::max_activations ? nil::actor::smp::count : Actor::max_activations);
            }

            return detail::do_with_actor_ref_impl<Actor, detail::ActorKey<Actor>>(
                key, next, [&func](auto const &impl) { return func(impl); });
        }

    public:
        /// Provides an intuitive function call-like API.
        /// The syntax `ref->msg(args...)` is equivalent to `ref.tell(actor::message::msg, args...)` but shorter.
        /// \returns Returns the remote actor's interface
        inline constexpr typename Actor::internal::template interface<actor_ref<Actor>> operator->() const {
            return typename Actor::internal::template interface<actor_ref<Actor>> {*this};
        }

        /// Enqueue a message to the [nil::actor::actor]() referenced by this [nil::actor::actor_ref]() instance
        /// \effects Creates the [nil::actor::actor]() if it doesn't exist
        /// \param message The message handler to enqueue
        /// \param args Arguments to pass to the message handler
        /// \returns A future representing the eventually returned value by the actor, or a failed future
        template<typename Handler, typename... Args>
        constexpr auto inline tell(Handler message, Args &&...args) const {
            return [this, message, args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                return visit([message, &args](auto const &impl) mutable {
                    return std::apply(
                        [&impl, message](auto &&...args) mutable {
                            return impl.tell(message, std::forward<Args>(args)...);
                        },
                        std::move(args));
                });
            }();
        }

        template<typename Handler, typename PackedArgs>
        constexpr auto inline tell_packed(Handler message, PackedArgs &&args) const {
            return visit([message, args = std::forward<PackedArgs>(args)](auto const &impl) mutable {
                return impl.tell_packed(message, std::forward<PackedArgs>(args));
            });
        }
    };

    /// A movable and copyable type-erased reference to a virtual actor.
    /// Useful when an [nil::actor::actor]() declares a message with an `actor_ref<itself>` as argument.
    /// Avoids incomplete type compiler error.
    /// \remarks Use only when needed, as this type introduces overhead
    /// \unique_name nil::actor::poly_actor_ref
    class poly_actor_ref {
        /// \exclude
        std::any opaque;

    public:
        template<template<typename> class Ref, typename Actor>
        constexpr poly_actor_ref(Ref<Actor> &&ref) :
            opaque(std::forward<Ref<Actor>>

                   (ref)) {
            static_assert(std::is_same_v<actor_ref<Actor>, Ref<Actor>>, "poly_actor_ref used with non actor_ref type");
        };

        template<template<typename> class Ref, typename Actor>
        constexpr poly_actor_ref(Ref<Actor> const &ref) : opaque(ref) {
            static_assert(std::is_same_v<actor_ref<Actor>, Ref<Actor>>, "poly_actor_ref used with non actor_ref type");
        };

        /// Cast this instance into a fully specified `actor_ref`
        /// \requires Actor shall be of type [nil::actor::actor]()
        /// \tparam Actor The type of [nil::actor::actor]() to reference
        /// \returns An [nil::actor::actor_ref]()
        template<typename Actor>
        auto as() {
            return std::any_cast<actor_ref<Actor>>(opaque);
        }
    };

    /// Create a reference to a virtual actor
    /// \tparam Actor The type of [nil::actor::actor]() to reference
    /// \requires Type `Actor` shall inherit from [nil::actor::actor]()
    /// \requires Type `KeyType` shall be of type `Actor::KeyType`
    /// \param key The primary key of the actor
    /// \returns An [nil::actor::actor_ref]()
    template<typename Actor, typename KeyType = typename Actor::KeyType>
    [[nodiscard]] constexpr inline actor_ref<Actor> get(KeyType &&key) noexcept {
        static_assert(std::is_constructible<detail::ActorKey<Actor>, KeyType &&>::value,
                      "The provided key is not compatible with the Actor");
        return actor_ref<Actor>(std::forward<KeyType>(key));
    }
}    // namespace nil::actor
