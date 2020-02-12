#pragma once

#include <nil/actor/detail/type_list.hpp>

#include <nil/actor/opencl/command.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            // signature for the function that is applied to output arguments
            template<class List>
            struct output_function_sig;

            template<class... Ts>
            struct output_function_sig<detail::type_list<Ts...>> {
                using type = std::function<message(Ts &...)>;
            };

            // derive signature of the command that handles the kernel execution
            template<class T, class List>
            struct command_sig;

            template<class T, class... Ts>
            struct command_sig<T, detail::type_list<Ts...>> {
                using type = opencl::command<T, Ts...>;
            };

            // derive type for a tuple matching the arguments as mem_refs
            template<class List>
            struct tuple_type_of;

            template<class... Ts>
            struct tuple_type_of<detail::type_list<Ts...>> {
                using type = std::tuple<Ts...>;
            };

        }    // namespace detail
    }        // namespace actor
}    // namespace nil
