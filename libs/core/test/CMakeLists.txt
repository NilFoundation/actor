include(CMTest)

cm_test_link_libraries(${CMAKE_WORKSPACE_NAME}_core)

if(NOT Boost_UNIT_TEST_FRAMEWORK_FOUND)
    cm_find_package(Boost REQUIRED COMPONENTS system unit_test_framework)
endif()

cm_test_link_libraries(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME}

                       ${Boost_LIBRARIES})

macro(define_core_test name)
    cm_test(NAME core_${name}_test SOURCES ${name}.cpp)

    target_include_directories(core_${name}_test PRIVATE
                               "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test/include>"
                               "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"

                               ${Boost_INCLUDE_DIRS})

    set_target_properties(core_${name}_test PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED TRUE)

    get_target_property(target_type Boost::unit_test_framework TYPE)
    if(target_type STREQUAL "SHARED_LIB")
        target_compile_definitions(core_${name}_test PRIVATE BOOST_TEST_DYN_LINK)
    elseif(target_type STREQUAL "STATIC_LIB")

    endif()
endmacro()

list(APPEND TESTS_NAMES
     actor_clock
     actor_factory
     actor_lifetime
     actor_pool
     actor_termination
     aout
     atom
     behavior
     blocking_actor
     bounds_checker
     broadcast_downstream_manager
     composable_behavior
     composition
     constructor_attach
     continuous_streaming
     custom_exception_handler
     deep_to_string
     delayed_send
     detached_actors
     dictionary
     drr_cached_queue
     drr_queue
     dynamic_spawn
     expected
     extract
     fifo_inbox
     function_view
     fused_streaming
     handles
     inbound_path
     inspector
     ipv4_address
     ipv4_subnet
     ipv6_address
     ipv6_subnet
     lifo_inbox
     local_group
     local_migration
     mailbox_element
     match
     message
     message_lifetime
     metaprogramming
     mock_streaming_classes
     native_streaming_classes
     optional
     or_else
     pipeline_streaming
     request_response
     request_timeout
     result
     selective_streaming
     sequencer
     serial_reply
     serialization
     simple_timeout
     splitter
     stateful_actor
     streambuf
     string_view
     sum_type
     sum_type_token
     task_queue
     thread_hook
     tick_emitter
     to_string
     type_erased_tuple
     typed_response_promise
     typed_spawn
     unit
     unordered_flat_map
     uri
     wdrr_dynamic_multiplexed_queue
     wdrr_fixed_multiplexed_queue)

foreach(TEST_NAME ${TESTS_NAMES})
    define_core_test(${TEST_NAME})
endforeach()