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

#include <cstddef>

#include <nil/mtl/extend.hpp>
#include <nil/mtl/message.hpp>
#include <nil/mtl/message_id.hpp>
#include <nil/mtl/ref_counted.hpp>
#include <nil/mtl/make_message.hpp>
#include <nil/mtl/message_view.hpp>
#include <nil/mtl/memory_managed.hpp>
#include <nil/mtl/type_erased_tuple.hpp>
#include <nil/mtl/actor_control_block.hpp>

#include <nil/mtl/intrusive/singly_linked.hpp>

#include <nil/mtl/meta/type_name.hpp>
#include <nil/mtl/meta/omittable_if_empty.hpp>

#include <nil/mtl/detail/disposer.hpp>
#include <nil/mtl/detail/tuple_vals.hpp>
#include <nil/mtl/detail/type_erased_tuple_view.hpp>

namespace nil {
    namespace mtl {

        class mailbox_element : public intrusive::singly_linked<mailbox_element>,
                                public memory_managed,
                                public message_view {
        public:
            using forwarding_stack = std::vector<strong_actor_ptr>;

            /// Source of this message and receiver of the final response.
            strong_actor_ptr sender;

            /// Denotes whether this an asynchronous message or a request.
            message_id mid;

            /// `stages.back()` is the next actor in the forwarding chain,
            /// if this is empty then the original sender receives the response.
            forwarding_stack stages;

            mailbox_element();

            mailbox_element(strong_actor_ptr &&x, message_id y, forwarding_stack &&z);

            ~mailbox_element() override;

            inline bool is_high_priority() const {
                return mid.category() == message_id::urgent_message_category;
            }

            mailbox_element(mailbox_element &&) = delete;
            mailbox_element(const mailbox_element &) = delete;
            mailbox_element &operator=(mailbox_element &&) = delete;
            mailbox_element &operator=(const mailbox_element &) = delete;
        };

        /// Corrects the message ID for down- and upstream messages to make sure the
        /// category for a `mailbox_element` matches its content.
        template<class...>
        struct mailbox_category_corrector {
            static constexpr message_id apply(message_id x) noexcept {
                return x;
            }
        };

        template<>
        struct mailbox_category_corrector<downstream_msg> {
            static constexpr message_id apply(message_id x) noexcept {
                return x.with_category(message_id::downstream_message_category);
            }
        };

        template<>
        struct mailbox_category_corrector<upstream_msg> {
            static constexpr message_id apply(message_id x) noexcept {
                return x.with_category(message_id::upstream_message_category);
            }
        };

        /// @relates mailbox_element
        template<class Inspector>
        typename Inspector::result_type inspect(Inspector &f, mailbox_element &x) {
            return f(meta::type_name("mailbox_element"), x.sender, x.mid, meta::omittable_if_empty(), x.stages,
                     x.content());
        }

        /// Encapsulates arbitrary data in a message element.
        template<class... Ts>
        class mailbox_element_vals final : public mailbox_element,
                                           public detail::tuple_vals_impl<type_erased_tuple, Ts...> {
        public:
            template<class... Us>
            mailbox_element_vals(strong_actor_ptr &&x0, message_id x1, forwarding_stack &&x2, Us &&... xs) :
                mailbox_element(std::move(x0), mailbox_category_corrector<Ts...>::apply(x1), std::move(x2)),
                detail::tuple_vals_impl<type_erased_tuple, Ts...>(std::forward<Us>(xs)...) {
                // nop
            }

            type_erased_tuple &content() override {
                return *this;
            }

            const type_erased_tuple &content() const override {
                return *this;
            }

            message move_content_to_message() override {
                message_factory f;
                auto &xs = this->data();
                return detail::apply_moved_args(f, detail::get_indices(xs), xs);
            }

            message copy_content_to_message() const override {
                message_factory f;
                auto &xs = this->data();
                return detail::apply_args(f, detail::get_indices(xs), xs);
            }

            void dispose() noexcept {
                this->deref();
            }
        };

        /// Provides a view for treating arbitrary data as message element.
        template<class... Ts>
        class mailbox_element_view final : public mailbox_element, public detail::type_erased_tuple_view<Ts...> {
        public:
            mailbox_element_view(strong_actor_ptr &&x0, message_id x1, forwarding_stack &&x2, Ts &... xs) :
                mailbox_element(std::move(x0), mailbox_category_corrector<Ts...>::apply(x1), std::move(x2)),
                detail::type_erased_tuple_view<Ts...>(xs...) {
                // nop
            }

            type_erased_tuple &content() override {
                return *this;
            }

            const type_erased_tuple &content() const override {
                return *this;
            }

            message move_content_to_message() override {
                message_factory f;
                auto &xs = this->data();
                return detail::apply_moved_args(f, detail::get_indices(xs), xs);
            }

            message copy_content_to_message() const override {
                message_factory f;
                auto &xs = this->data();
                return detail::apply_args(f, detail::get_indices(xs), xs);
            }
        };

        /// @relates mailbox_element
        using mailbox_element_ptr = std::unique_ptr<mailbox_element, detail::disposer>;

        /// @relates mailbox_element
        mailbox_element_ptr make_mailbox_element(strong_actor_ptr sender, message_id id,
                                                 mailbox_element::forwarding_stack stages, message msg);

        /// @relates mailbox_element
        template<class T, class... Ts>
        typename std::enable_if<!std::is_same<typename std::decay<T>::type, message>::value || (sizeof...(Ts) > 0),
                                mailbox_element_ptr>::type
            make_mailbox_element(strong_actor_ptr sender, message_id id, mailbox_element::forwarding_stack stages,
                                 T &&x, Ts &&... xs) {
            using impl = mailbox_element_vals<
                typename unbox_message_element<typename detail::strip_and_convert<T>::type>::type,
                typename unbox_message_element<typename detail::strip_and_convert<Ts>::type>::type...>;
            auto ptr = new impl(std::move(sender), id, std::move(stages), std::forward<T>(x), std::forward<Ts>(xs)...);
            return mailbox_element_ptr {ptr};
        }

    }    // namespace mtl
}    // namespace nil
