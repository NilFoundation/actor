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

#include <nil/mtl/uniform_type_info_map.hpp>

#include <ios>    // std::ios_base::failure
#include <array>
#include <tuple>
#include <limits>
#include <string>
#include <vector>
#include <cstring>    // memcmp
#include <algorithm>
#include <type_traits>

#include <nil/mtl/abstract_group.hpp>
#include <nil/mtl/actor_cast.hpp>
#include <nil/mtl/actor_factory.hpp>
#include <nil/mtl/actor_system.hpp>
#include <nil/mtl/actor_system_config.hpp>
#include <nil/mtl/downstream_msg.hpp>
#include <nil/mtl/duration.hpp>
#include <nil/mtl/group.hpp>
#include <nil/mtl/locks.hpp>
#include <nil/mtl/logger.hpp>
#include <nil/mtl/message.hpp>
#include <nil/mtl/message_builder.hpp>
#include <nil/mtl/proxy_registry.hpp>
#include <nil/mtl/string_algorithms.hpp>
#include <nil/mtl/timespan.hpp>
#include <nil/mtl/timestamp.hpp>
#include <nil/mtl/type_nr.hpp>
#include <nil/mtl/upstream_msg.hpp>

#include <nil/mtl/detail/safe_equal.hpp>
#include <nil/mtl/detail/scope_guard.hpp>
#include <nil/mtl/detail/shared_spinlock.hpp>

namespace nil {
    namespace mtl {

        const char *numbered_type_names[] = {"@actor",
                                             "@actorvec",
                                             "@addr",
                                             "@addrvec",
                                             "@atom",
                                             "@bytebuf",
                                             "@charbuf",
                                             "@config_value",
                                             "@down",
                                             "@downstream_msg",
                                             "@duration",
                                             "@error",
                                             "@exit",
                                             "@group",
                                             "@group_down",
                                             "@i16",
                                             "@i32",
                                             "@i64",
                                             "@i8",
                                             "@ldouble",
                                             "@message",
                                             "@message_id",
                                             "@node",
                                             "@open_stream_msg",
                                             "@str",
                                             "@strmap",
                                             "@strong_actor_ptr",
                                             "@strset",
                                             "@strvec",
                                             "@timeout",
                                             "@timespan",
                                             "@timestamp",
                                             "@u16",
                                             "@u16str",
                                             "@u32",
                                             "@u32str",
                                             "@u64",
                                             "@u8",
                                             "@unit",
                                             "@upstream_msg",
                                             "@weak_actor_ptr",
                                             "bool",
                                             "double",
                                             "float"};

        namespace {

            using builtins = std::array<uniform_type_info_map::value_factory_kvp, type_nrs - 1>;

            void fill_builtins(builtins &, detail::type_list<>, size_t) {
                // end of recursion
            }

            template<class List>
            void fill_builtins(builtins &arr, List, size_t pos) {
                using type = typename detail::tl_head<List>::type;
                typename detail::tl_tail<List>::type next;
                arr[pos].first = numbered_type_names[pos];
                arr[pos].second = &make_type_erased_value<type>;
                fill_builtins(arr, next, pos + 1);
            }

        }    // namespace

        type_erased_value_ptr uniform_type_info_map::make_value(uint16_t nr) const {
            return builtin_[nr - 1].second();
        }

        type_erased_value_ptr uniform_type_info_map::make_value(const std::string &x) const {
            auto pred = [&](const value_factory_kvp &kvp) { return kvp.first == x; };
            auto e = builtin_.end();
            auto i = std::find_if(builtin_.begin(), e, pred);
            if (i != e)
                return i->second();
            auto &custom_names = system().config().value_factories_by_name;
            auto j = custom_names.find(x);
            if (j != custom_names.end())
                return j->second();
            return nullptr;
        }

        type_erased_value_ptr uniform_type_info_map::make_value(const std::type_info &x) const {
            auto &custom_by_rtti = system().config().value_factories_by_rtti;
            auto i = custom_by_rtti.find(std::type_index(x));
            if (i != custom_by_rtti.end())
                return i->second();
            return nullptr;
        }

        const std::string &uniform_type_info_map::portable_name(uint16_t nr, const std::type_info *ti) const {
            if (nr != 0)
                return builtin_names_[nr - 1];
            if (ti == nullptr)
                return default_type_name_;
            auto &custom_names = system().config().type_names_by_rtti;
            auto i = custom_names.find(std::type_index(*ti));
            if (i != custom_names.end())
                return i->second;
            return default_type_name_;
        }

        uniform_type_info_map::uniform_type_info_map(actor_system &sys) : system_(sys), default_type_name_("???") {
            sorted_builtin_types list;
            fill_builtins(builtin_, list, 0);
            for (size_t i = 0; i < builtin_names_.size(); ++i)
                builtin_names_[i] = numbered_type_names[i];
        }

    }    // namespace mtl
}    // namespace nil
