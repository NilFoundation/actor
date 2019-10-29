cmake_minimum_required(VERSION 3.5)

cm_find_package(CM)
include(CMDeploy)
include(CMSetupVersion)

if(NOT Boost_FOUND)
    cm_find_package(Boost COMPONENTS thread)
endif()

cm_project(io WORKSPACE_NAME ${CMAKE_WORKSPACE_NAME})

cm_setup_version(VERSION 0.16.0)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB_RECURSE ${CURRENT_PROJECT_NAME}_HEADERS "include/nil/mtl/*.hpp")

# list cpp files excluding platform-dependent files
set(${CURRENT_PROJECT_NAME}_SOURCES
    src/abstract_broker.cpp
    src/acceptor.cpp
    src/acceptor_manager.cpp
    src/basp_broker.cpp
    src/broker.cpp
    src/connection_helper.cpp
    src/datagram_handler.cpp
    src/datagram_manager.cpp
    src/datagram_servant.cpp
    src/datagram_servant_impl.cpp
    src/default_multiplexer.cpp
    src/doorman.cpp
    src/doorman_impl.cpp
    src/event_handler.cpp
    src/header.cpp
    src/instance.cpp
    src/interfaces.cpp
    src/ip_endpoint.cpp
    src/manager.cpp
    src/message_queue.cpp
    src/message_type.cpp
    src/middleman.cpp
    src/middleman_actor.cpp
    src/middleman_actor_impl.cpp
    src/multiplexer.cpp
    src/native_socket.cpp
    src/pipe_reader.cpp
    src/protocol.cpp
    src/receive_buffer.cpp
    src/routing_table.cpp
    src/scribe.cpp
    src/scribe_impl.cpp
    src/socket_guard.cpp
    src/stream.cpp
    src/stream_manager.cpp
    src/tcp.cpp
    src/test_multiplexer.cpp
    src/udp.cpp
    src/worker.cpp
    )

add_library(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} ${${CURRENT_PROJECT_NAME}_HEADERS}
            ${${CURRENT_PROJECT_NAME}_SOURCES})
set_target_properties(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PROPERTIES
                      LINKER_LANGUAGE CXX
                      EXPORT_NAME ${CURRENT_PROJECT_NAME}
                      CXX_STANDARD 11
                      CXX_STANDARD_REQUIRED TRUE)

target_link_libraries(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PRIVATE
                      ${CMAKE_WORKSPACE_NAME}::core

                      ${Boost_LIBRARIES})

target_include_directories(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PRIVATE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>

                           ${Boost_INCLUDE_DIRS})

cm_deploy(TARGETS ${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME}
          INCLUDE include
          NAMESPACE ${CMAKE_WORKSPACE_NAME}::)

if(APPLE OR NOT ${CMAKE_TARGET_ARCHITECTURE} STREQUAL ${CMAKE_HOST_SYSTEM_PROCESSOR})
    set_target_properties(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PROPERTIES
                          XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${APPLE_CODE_SIGN_IDENTITY}"
                          XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}")
endif()

if(BUILD_TESTS)
    add_subdirectory(test)
endif()