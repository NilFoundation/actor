cmake_minimum_required(VERSION 3.12.4)

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0057 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake"
     "${CMAKE_CURRENT_LIST_DIR}/cmake/packages"
     "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/share/modules/cmake")

cm_find_package(CM)

include(CMConfig)
include(CMDeploy)
include(CMSetupVersion)

if(NOT CMAKE_WORKSPACE_NAME OR NOT ("${CMAKE_WORKSPACE_NAME}" STREQUAL "actor"))
    cm_workspace(actor)
endif()

macro(cm_find_package NAME)
    if(NOT "${NAME}" MATCHES "^${CMAKE_WORKSPACE_NAME}_.*$" AND NOT "${NAME}" STREQUAL CM)
        find_package(${ARGV})
    else()
        set(${ARGV0}_FOUND ON CACHE BOOL "")
    endif()
endmacro()

cm_project(cluster WORKSPACE_NAME ${CMAKE_WORKSPACE_NAME} LANGUAGES CXX)

cm_setup_version(VERSION 0.1.0)

cm_find_package(${CMAKE_WORKSPACE_NAME}_core REQUIRED)
cm_find_package(${CMAKE_WORKSPACE_NAME}_network REQUIRED)

# Compiler checks
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB_RECURSE ${CURRENT_PROJECT_NAME}_HEADERS "include/nil/actor/*.hpp")

# list cpp files excluding platform-dependent files
set(${CURRENT_PROJECT_NAME}_SOURCES 
    src/server.cpp
    src/membership.cpp
    src/node.cpp
    src/handshake.cpp
    src/cluster.cpp)

add_library(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME}
            ${${CURRENT_PROJECT_NAME}_HEADERS}
            ${${CURRENT_PROJECT_NAME}_SOURCES})

if(${COMPILER_SUPPORTS_MARCH_NATIVE})
    target_compile_options(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PRIVATE -march=native)
endif()


set_target_properties(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PROPERTIES
                      LINKER_LANGUAGE CXX
                      EXPORT_NAME ${CURRENT_PROJECT_NAME}
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED TRUE)

target_link_libraries(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PRIVATE
                      ${CMAKE_WORKSPACE_NAME}::core
                      ${CMAKE_WORKSPACE_NAME}::network

                      crypto3::hash)

target_include_directories(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PRIVATE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

if(APPLE OR NOT ${CMAKE_TARGET_ARCHITECTURE} STREQUAL ${CMAKE_HOST_SYSTEM_PROCESSOR})
    set_target_properties(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PROPERTIES
                          XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${APPLE_CODE_SIGN_IDENTITY}"
                          XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}")
endif()


cm_deploy(TARGETS ${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME}
          INCLUDE include
          NAMESPACE ${CMAKE_WORKSPACE_NAME}::)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()
