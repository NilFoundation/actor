cmake_minimum_required(VERSION 3.2)

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0057 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake"
     "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/share/modules/cmake")

include(CMConfig)
include(CMSetupVersion)

cm_workspace(actor LANGUAGES C CXX)

macro(cm_find_package NAME)
    if(NOT "${NAME}" MATCHES "^${CMAKE_WORKSPACE_NAME}_.*$" AND NOT "${NAME}" STREQUAL CM)
        find_package(${ARGV})
    else()
        set(${ARGV0}_FOUND ON CACHE BOOL "")
    endif()
endmacro()

cm_setup_version(VERSION 0.18.0 PREFIX ${CMAKE_WORKSPACE_NAME})

include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(TargetArchitecture)

option(BUILD_WITH_CCACHE "Build with ccache usage" TRUE)
option(BUILD_TESTS "Build tests" FALSE)

set(BUILD_WITH_TARGET_ARCHITECTURE "" CACHE STRING "Target build architecture")

if(BUILD_WITH_TARGET_ARCHITECTURE)
    set(CMAKE_TARGET_ARCHITECTURE ${BUILD_WITH_TARGET_ARCHITECTURE})
else()
    target_architecture(CMAKE_TARGET_ARCHITECTURE)
endif()

# Be nice to VIM users and Clang tools.
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Silence policy CMP0042 warning by enabling RPATH explicitly.
if(APPLE AND NOT DEFINED CMAKE_MACOSX_RPATH)
    set(CMAKE_MACOSX_RPATH true)
endif()

# add helper target that simplifies re-running configure
add_custom_target(configure COMMAND ${CMAKE_CURRENT_BINARY_DIR}/config.status)

################################################################################
#                              utility functions                               #
################################################################################

# Appends `str` to the variable named `var` with a whitespace as separator.
# Suppresses a leading whitespace if the variable is empty and does nothing if
# `str` is empty.
function(build_string var str)
    if(NOT str STREQUAL "")
        if("${${var}}" STREQUAL "")
            set("${var}" "${str}" PARENT_SCOPE)
        else()
            set("${var}" "${${var}} ${str}" PARENT_SCOPE)
        endif()
    endif()
endfunction(build_string)

# Forces `var` to 'no' if the content of the variables evaluates to false.
function(pretty_no var)
    if(NOT "${${var}}")
        set("${var}" no PARENT_SCOPE)
    endif()
endfunction(pretty_no)

# Forces `var` to 'yes' if the content of the variables evaluates to false.
function(pretty_yes var)
    if("${${var}}")
        set("${var}" yes PARENT_SCOPE)
    endif()
endfunction(pretty_yes)

################################################################################
#                      enable ccache if required by user                       #
################################################################################

if(BUILD_WITH_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        message(STATUS "Using ccache command: ${CCACHE_PROGRAM}")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    else()
        message(STATUS "Unable to find ccache")
    endif()
endif(BUILD_WITH_CCACHE)

################################################################################
#      make sure config parameters are printed with yes or no in summary       #
################################################################################

pretty_yes("ACTOR_FORCE_NO_EXCEPTIONS")

pretty_no("ACTOR_ENABLE_RUNTIME_CHECKS")
pretty_no("ACTOR_NO_MEM_MANAGEMENT")
pretty_no("ACTOR_NO_EXCEPTIONS")
pretty_no("ACTOR_BUILD_STATIC_ONLY")
pretty_no("ACTOR_BUILD_STATIC")
pretty_no("ACTOR_NO_OPENCL")
pretty_no("ACTOR_NO_OPENSSL")
pretty_no("ACTOR_NO_PYTHON")
pretty_no("ACTOR_NO_TOOLS")
pretty_no("ACTOR_NO_SUMMARY")

if(NOT ACTOR_NO_IO)
    set(ACTOR_NO_IO no)
else()
    set(ACTOR_NO_TOOLS yes)
    set(ACTOR_NO_PYTHON yes)
endif()

################################################################################
#   set output paths for binaries and libraries if not provided by the user    #
################################################################################

# prohibit in-source builds
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use "
            "./configure to choose a build directory and "
            "initialize the build configuration.")
endif()
# set module path appropriately
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# set binary output path if not defined by user
if(EXECUTABLE_OUTPUT_PATH STREQUAL "")
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin")
endif()
# set library output path if not defined by user, but always set
# library output path to binary output path for Xcode projects
if(CMAKE_GENERATOR STREQUAL "Xcode")
    set(LIBRARY_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}")
elseif(LIBRARY_OUTPUT_PATH STREQUAL "")
    set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()


################################################################################
#                                compiler setup                                #
################################################################################

# check for g++ >= 4.8 or clang++ > = 3.2
if(NOT WIN32 AND NOT ACTOR_NO_COMPILER_CHECK AND NOT CMAKE_CROSSCOMPILING)
    try_run(ProgramResult
            CompilationSucceeded
            "${CMAKE_CURRENT_BINARY_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_compiler_version.cpp"
            RUN_OUTPUT_VARIABLE CompilerVersion)
    if(NOT CompilationSucceeded OR NOT ProgramResult EQUAL 0)
        message(FATAL_ERROR "Cannot determine compiler version")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        if(CompilerVersion VERSION_GREATER 4.7)
            message(STATUS "Found g++ version ${CompilerVersion}")
        else()
            message(FATAL_ERROR "g++ >= 4.8 required (found: ${CompilerVersion})")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(CompilerVersion VERSION_GREATER 3.1)
            message(STATUS "Found clang++ version ${CompilerVersion}")
        else()
            message(FATAL_ERROR "clang++ >= 3.2 required (found: ${CompilerVersion})")
        endif()
    else()
        message(FATAL_ERROR "Your C++ compiler does not support C++11 "
                "or is not supported")
    endif()
endif()
# set optional build flags
# increase max. template depth on GCC and Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
   OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    build_string("EXTRA_FLAGS" "-ftemplate-depth=512 -ftemplate-backtrace-limit=0")
endif()
# explicitly disable obnoxious GCC warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    build_string("EXTRA_FLAGS" "-Wno-missing-field-initializers")
endif()
# add "-Werror" flag if --pedantic-build is used
if(ACTOR_CXX_WARNINGS_AS_ERRORS)
    build_string("EXTRA_FLAGS" "-Werror")
endif()
# set compiler flags for GCOV if requested
if(ACTOR_ENABLE_GCOV)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(NO_INLINE "-fno-inline")
    else()
        set(NO_INLINE "-fno-inline -fno-inline-small-functions -fno-default-inline")
    endif()
    build_string("EXTRA_FLAGS" "-fprofile-arcs -ftest-coverage ${NO_INLINE}")
endif()
# set -fno-exception if requested
if(ACTOR_FORCE_NO_EXCEPTIONS)
    build_string("EXTRA_FLAGS" "-fno-exceptions")
endif()
# enable a ton of warnings if --more-clang-warnings is used
if(ACTOR_MORE_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(WFLAGS "-Weverything -Wno-c++98-compat -Wno-padded "
            "-Wno-documentation-unknown-command -Wno-exit-time-destructors "
            "-Wno-global-constructors -Wno-missing-prototypes "
            "-Wno-c++98-compat-pedantic -Wno-unused-member-function "
            "-Wno-unused-const-variable -Wno-switch-enum "
            "-Wno-abstract-vbase-init "
            "-Wno-missing-noreturn -Wno-covered-switch-default")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(WFLAGS "-Waddress -Wall -Warray-bounds "
            "-Wattributes -Wbuiltin-macro-redefined -Wcast-align "
            "-Wcast-qual -Wchar-subscripts -Wclobbered -Wcomment "
            "-Wconversion -Wconversion-null -Wcoverage-mismatch "
            "-Wcpp -Wdelete-non-virtual-dtor -Wdeprecated "
            "-Wdeprecated-declarations -Wdiv-by-zero -Wdouble-promotion "
            "-Wempty-body -Wendif-labels -Wenum-compare -Wextra "
            "-Wfloat-equal -Wformat -Wfree-nonheap-object "
            "-Wignored-qualifiers -Winit-self "
            "-Winline -Wint-to-pointer-cast -Winvalid-memory-model "
            "-Winvalid-offsetof -Wlogical-op -Wmain -Wmaybe-uninitialized "
            "-Wmissing-braces -Wmultichar "
            "-Wnarrowing -Wnoexcept -Wnon-template-friend "
            "-Wnon-virtual-dtor -Wnonnull -Woverflow "
            "-Woverlength-strings -Wparentheses "
            "-Wpmf-conversions -Wpointer-arith -Wreorder "
            "-Wreturn-type -Wsequence-point "
            "-Wsign-compare -Wswitch -Wtype-limits -Wundef "
            "-Wuninitialized -Wunused -Wvla -Wwrite-strings")
    endif()
    # convert CMake list to a single string, erasing the ";" separators
    string(REPLACE ";" "" WFLAGS_STR ${WFLAGS})
    build_string("EXTRA_FLAGS" "${WFLAGS_STR}")
endif()

# add -stdlib=libc++ when using Clang if possible
if(NOT ACTOR_NO_AUTO_LIBCPP AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXXFLAGS_BACKUP "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "-stdlib=libc++")
    try_run(ProgramResult
            CompilationSucceeded
            "${CMAKE_CURRENT_BINARY_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_compiler_version.cpp"
            RUN_OUTPUT_VARIABLE CompilerVersion)
    if(NOT CompilationSucceeded OR NOT ProgramResult EQUAL 0)
        message(STATUS "Use clang with GCC' libstdc++")
    else()
        message(STATUS "Automatically added '-stdlib=libc++' flag "
                "(ACTOR_NO_AUTO_LIBCPP not defined)")
        build_string("EXTRA_FLAGS" "-stdlib=libc++")
    endif()
    # restore CXX flags
    set(CMAKE_CXX_FLAGS "${CXXFLAGS_BACKUP}")
endif()
# enable address sanitizer if requested by the user
if(ACTOR_ENABLE_ADDRESS_SANITIZER)
    # check whether address sanitizer is available
    set(CXXFLAGS_BACKUP "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
    try_run(ProgramResult
            CompilationSucceeded
            "${CMAKE_CURRENT_BINARY_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_compiler_version.cpp")
    if(NOT CompilationSucceeded)
        message(WARNING "Address Sanitizer is not available on selected compiler")
    else()
        message(STATUS "Enable Address Sanitizer")
        build_string("EXTRA_FLAGS" "-fsanitize=address -fno-omit-frame-pointer")
    endif()
    # restore CXX flags
    set(CMAKE_CXX_FLAGS "${CXXFLAGS_BACKUP}")
endif(ACTOR_ENABLE_ADDRESS_SANITIZER)
# -pthread is ignored on MacOSX but required on other platforms
if(NOT APPLE AND NOT WIN32)
    build_string("EXTRA_FLAGS" "-pthread")
endif()
# -fPIC generates warnings on MinGW and Cygwin plus extra setup steps needed on MinGW
if(MINGW)
    add_definitions(-D_WIN32_WINNT=0x0600)
    add_definitions(-DWIN32)
    include(GenerateExportHeader)
    list(APPEND ACTOR_EXTRA_LDFLAGS -lws2_32 -liphlpapi -lpsapi)
    # build static to avoid runtime dependencies to GCC libraries
    build_string("EXTRA_FLAGS" "-static")
elseif(CYGWIN)
    build_string("EXTRA_FLAGS" "-U__STRICT_ANSI__")
else()
    build_string("EXTRA_FLAGS" "-fPIC")
endif()
if(WIN32)
    list(APPEND ACTOR_EXTRA_LDFLAGS ws2_32 iphlpapi)
endif()
# iOS support
if(ACTOR_OSX_SYSROOT)
    set(CMAKE_OSX_SYSROOT "${ACTOR_OSX_SYSROOT}")
endif()
if(BOOST_OS_IOS_AVAILABLE_DEPLOYMENT_TARGET)
    if(ACTOR_OSX_SYSROOT STREQUAL "iphonesimulator")
        build_string("EXTRA_FLAGS"
                     "-mios-simulator-version-min=${BOOST_OS_IOS_AVAILABLE_DEPLOYMENT_TARGET}")
    else()
        build_string("EXTRA_FLAGS"
                     "-miphoneos-version-min=${BOOST_OS_IOS_AVAILABLE_DEPLOYMENT_TARGET}")
    endif()
endif()
# check if the user provided CXXFLAGS, set defaults otherwise
if(NOT CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "-Wextra -Wall -pedantic ${EXTRA_FLAGS}")
endif()
if(NOT CMAKE_CXX_FLAGS_DEBUG)
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
endif()
if(NOT CMAKE_CXX_FLAGS_MINSIZEREL)
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
endif()
if(NOT CMAKE_CXX_FLAGS_RELEASE)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()
if(NOT CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -fno-omit-frame-pointer")
endif()
# set build default build type to RelWithDebInfo if not set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
# needed by subprojects
if(DEFINED CMAKE_LD_LIBS)
    list(APPEND ${CMAKE_LD_LIBS})
endif()

################################################################################
#                                setup logging                                 #
################################################################################

if(NOT ACTOR_LOG_LEVEL)
    set(ACTOR_LOG_LEVEL "-1")
endif()

################################################################################
#                           setup for install target                           #
################################################################################

# install includes from test
install(DIRECTORY test/include/nil DESTINATION include FILES_MATCHING PATTERN "*.hpp")

# process cmake_uninstall.cmake.in
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
# add uninstall target
add_custom_target(uninstall
                  COMMAND "${CMAKE_COMMAND}" -P
                  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

################################################################################
#                                 add targets                                  #
################################################################################

add_subdirectories(${CMAKE_CURRENT_LIST_DIR}/libs/crypto)
add_subdirectories(${CMAKE_CURRENT_LIST_DIR}/libs)

# -- Fetch branch name and SHA if available ------------------------------------

if(EXISTS "release.txt")
    file(READ "release.txt" ACTOR_VESION)
else()
    set(ACTOR_RELEASE "${ACTOR_VERSION}")
    cm_find_package(Git QUIET)
    if(GIT_FOUND)
        # retrieve current branch name for ACTOR
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                        RESULT_VARIABLE gitBranchResult
                        OUTPUT_VARIABLE gitBranch
                        ERROR_QUIET
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        # retrieve current SHA1 hash for ACTOR
        execute_process(COMMAND ${GIT_EXECUTABLE} log --pretty=format:%h -n 1
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                        RESULT_VARIABLE gitShaResult
                        OUTPUT_VARIABLE gitSha
                        ERROR_QUIET
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(gitBranchResult EQUAL 0 AND gitShaResult EQUAL 0)
            # generate semantic ACTOR version for manual
            set(ACTOR_RELEASE "${ACTOR_VERSION}+exp.sha.${gitSha}")
            # check whether we're building the manual for a stable detach
            if(gitBranch STREQUAL "master")
                # retrieve current tag name for ACTOR
                execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --contains ${gitSha}
                                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                                RESULT_VARIABLE gitTagResult
                                OUTPUT_VARIABLE gitTag
                                ERROR_QUIET
                                OUTPUT_STRIP_TRAILING_WHITESPACE)
                # tags indicate stable releases -> use tag name as detach version
                if(gitTagResult EQUAL 0)
                    set(ACTOR_RELEASE "${gitTag}")
                endif()
            endif()
        endif()
    endif()
endif()

message(STATUS "Set release version for all documentation to ${ACTOR_RELEASE}.")

# -- Setup for building manual and API documentation ---------------------------

# we need the examples and some headers relative to the build/doc/tex directory
file(COPY libs/core/include/nil/actor/exit_reason.hpp DESTINATION core/nil/actor/)
file(COPY libs/core/include/nil/actor/sec.hpp DESTINATION core/nil/actor/)

add_subdirectory(doc)

################################################################################
#                     Add additional project files to GUI                      #
################################################################################

file(GLOB_RECURSE script_files "scripts/*")
add_custom_target(gui_dummy SOURCES configure ${script_files})

################################################################################
#                                print summary                                 #
################################################################################

# set human-readable representation for log level
set(LOG_LEVEL_STR "none")
if(ACTOR_LOG_LEVEL)
    if(${ACTOR_LOG_LEVEL} EQUAL 0)
        set(LOG_LEVEL_STR "ERROR")
    elseif(${ACTOR_LOG_LEVEL} EQUAL 1)
        set(LOG_LEVEL_STR "WARNING")
    elseif(${ACTOR_LOG_LEVEL} EQUAL 2)
        set(LOG_LEVEL_STR "INFO")
    elseif(${ACTOR_LOG_LEVEL} EQUAL 3)
        set(LOG_LEVEL_STR "DEBUG")
    elseif(${ACTOR_LOG_LEVEL} EQUAL 4)
        set(LOG_LEVEL_STR "TRACE")
    else()
        set(LOG_LEVEL_STR "invalid")
    endif()
endif()
# little helper macro to invert a boolean
macro(invertyesno in out)
    if(${in})
        set(${out} no)
    else()
        set(${out} yes)
    endif()
endmacro()
# invert ACTOR_NO_* variables for nicer output
invertyesno(ACTOR_NO_IO ACTOR_BUILD_IO)
invertyesno(ACTOR_NO_EXAMPLES ACTOR_BUILD_EXAMPLES)
invertyesno(ACTOR_NO_TOOLS ACTOR_BUILD_TOOLS)
invertyesno(ACTOR_NO_UNIT_TESTS ACTOR_BUILD_TESTS)
invertyesno(ACTOR_NO_EXCEPTIONS ACTOR_BUILD_WITH_EXCEPTIONS)
invertyesno(ACTOR_NO_MEM_MANAGEMENT ACTOR_BUILD_MEM_MANAGEMENT)
invertyesno(ACTOR_NO_BENCHMARKS ACTOR_BUILD_BENCHMARKS)
invertyesno(ACTOR_NO_OPENCL ACTOR_BUILD_OPENCL)
invertyesno(ACTOR_NO_OPENSSL ACTOR_BUILD_OPENSSL)
invertyesno(ACTOR_NO_PYTHON ACTOR_BUILD_PYTHON)
# collect all compiler flags
string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPER_BUILD_TYPE)
set(ALL_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${UPPER_BUILD_TYPE}}")
set(ALL_LD_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ACTOR_EXTRA_LDFLAGS}")
string(STRIP "${ALL_LD_FLAGS}" ALL_LD_FLAGS)
# done