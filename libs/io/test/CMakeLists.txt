include(CMTest)

cm_test_link_libraries(${CMAKE_WORKSPACE_NAME}_io)

if(NOT Boost_UNIT_TEST_FRAMEWORK_FOUND)
    cm_find_package(Boost REQUIRED COMPONENTS system unit_test_framework)
endif()

cm_test_link_libraries(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME}

                       ${CMAKE_WORKSPACE_NAME}_core

                       ${Boost_LIBRARIES})

macro(define_network_test name)
    cm_test(NAME io_${name}_test SOURCES ${name}.cpp)

    target_include_directories(io_${name}_test PRIVATE
                               "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test/include>"
                               "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"

                               ${Boost_INCLUDE_DIRS})

    set_target_properties(io_${name}_test PROPERTIES
                          CXX_STANDARD 17
                          CXX_STANDARD_REQUIRED TRUE)

    get_target_property(target_type Boost::unit_test_framework TYPE)
    if(target_type STREQUAL "SHARED_LIB")
        target_compile_definitions(io_${name}_test PRIVATE BOOST_TEST_DYN_LINK)
    elseif(target_type STREQUAL "STATIC_LIB")

    endif()
endmacro()

list(APPEND TESTS_NAMES
     triggering
     default_multiplexer
     remote_group
     basp_broker
     typed_remote_actor
     automatic_connection
     #     remote_actor_udp
     receive_buffer
     unpublish
     typed_broker
     ip_endpoint
     dynamic_broker
     #     basp_udp
     http_broker
     remote_spawn
     remote_actor)

foreach(TEST_NAME ${TESTS_NAMES})
    define_network_test(${TEST_NAME})
endforeach()
