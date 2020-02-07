cmake_minimum_required(VERSION 3.5)

cm_find_package(CM)
include(CMDeploy)
include(CMSetupVersion)

if(NOT Boost_THREAD_FOUND)
    cm_find_package(Boost COMPONENTS thread)
endif()

cm_project(network WORKSPACE_NAME ${CMAKE_WORKSPACE_NAME})

cm_setup_version(VERSION 0.16.0)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB_RECURSE ${CURRENT_PROJECT_NAME}_HEADERS "include/nil/mtl/*.hpp")

# list cpp files excluding platform-dependent files
set(${CURRENT_PROJECT_NAME}_SOURCES
    src/basp/connection_state_strings.cpp
    src/basp/ec_strings.cpp
    src/basp/message_type_strings.cpp
    src/basp/operation_strings.cpp
    src/network/backend/test.cpp
    src/network/endpoint_manager_queue.cpp
    src/network/middleman.cpp
    src/network/middleman_backend.cpp
    src/network/packet_writer.cpp
    src/actor_proxy_impl.cpp
    src/application.cpp
    src/convert_ip_endpoint.cpp
    src/datagram_socket.cpp
    src/defaults.cpp
    src/endpoint_manager.cpp
    src/header.cpp
    src/host.cpp
    src/ip.cpp
    src/message_queue.cpp
    src/multiplexer.cpp
    src/network_socket.cpp
    src/pipe_socket.cpp
    src/pollset_updater.cpp
    src/socket.cpp
    src/socket_manager.cpp
    src/stream_socket.cpp
    src/tcp_accept_socket.cpp
    src/tcp_stream_socket.cpp
    src/udp_datagram_socket.cpp
    src/worker.cpp)

add_library(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} ${${CURRENT_PROJECT_NAME}_HEADERS}
            ${${CURRENT_PROJECT_NAME}_SOURCES})
set_target_properties(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PROPERTIES
                      LINKER_LANGUAGE CXX
                      EXPORT_NAME ${CURRENT_PROJECT_NAME}
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED TRUE)

target_link_libraries(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PRIVATE
                      ${CMAKE_WORKSPACE_NAME}::core

                      marshalling

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