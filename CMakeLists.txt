cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0057 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake"
     "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/share/modules/cmake")

include(CMConfig)
include(CMSetupVersion)

cm_workspace(actor LANGUAGES CXX)

macro(cm_find_package NAME)
    if(NOT "${NAME}" MATCHES "^${CMAKE_WORKSPACE_NAME}_.*$" AND NOT "${NAME}" STREQUAL CM)
        find_package(${ARGV})
    else()
        set(${ARGV0}_FOUND ON CACHE BOOL "")
    endif()
endmacro()

include(CheckCSourceCompiles)
include(CheckCSourceRuns)

option(BUILD_WITH_CCACHE "Build with ccache usage" TRUE)
option(BUILD_TESTS "Build tests" FALSE)

if(UNIX AND BUILD_WITH_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)
endif()

option(BUILD_APPS "Enable application targets." FALSE)
option(BUILD_EXAMPLES "Enable demonstration targets." FALSE)
option(BUILD_DOCS "Enable documentation targets." FALSE)

set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_LIST_DIR}/docs" CACHE STRING "Specify doxygen output directory")

#
# Code generation helpers.
#

function(seastar_generate_swagger)
    set(one_value_args TARGET VAR IN_FILE OUT_DIR)
    cmake_parse_arguments(args "" "${one_value_args}" "" ${ARGN})
    get_filename_component(in_file_name ${args_IN_FILE} NAME)
    set(generator ${BUILD_WITH_SOURCE_DIR}/scripts/seastar-json2code.py)
    set(header_out ${args_OUT_DIR}/${in_file_name}.hh)
    set(source_out ${args_OUT_DIR}/${in_file_name}.cc)

    add_custom_command(
            DEPENDS
            ${args_IN_FILE}
            ${generator}
            OUTPUT ${header_out} ${source_out}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${args_OUT_DIR}
            COMMAND ${generator} --create-cc -f ${args_IN_FILE} -o ${header_out})

    add_custom_target(${args_TARGET}
                      DEPENDS
                      ${header_out}
                      ${source_out})

    set(${args_VAR} ${header_out} ${source_out} PARENT_SCOPE)
endfunction()


add_subdirectories(${CMAKE_CURRENT_LIST_DIR}/libs/crypto)

if(NOT (UNIX AND (CMAKE_SYSTEM_NAME STREQUAL "Linux")))
    add_subdirectories(${CMAKE_CURRENT_LIST_DIR}/libs/shim)
endif()

add_subdirectories(${CMAKE_CURRENT_LIST_DIR}/libs)

#
# Demonstrations.
#

if(BUILD_EXAMPLES)
    if(BUILD_WITH_EXCLUDE_EXAMPLES_FROM_ALL)
        set(exclude EXCLUDE_FROM_ALL)
    else()
        set(exclude "")
    endif()

    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/examples/utils ${exclude})
endif()

#
# Documentation.
#

if(BUILD_DOCS)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen/actor.doxyfile.in
                   ${CMAKE_CURRENT_BINARY_DIR}/actor.doxyfile @ONLY)
endif()

#
# Applications.
#

if(BUILD_APPS)
    if(BUILD_WITH_EXCLUDE_APPS_FROM_ALL)
        set(exclude EXCLUDE_FROM_ALL)
    else()
        set(exclude "")
    endif()

    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/examples/apps ${exclude})
endif()
