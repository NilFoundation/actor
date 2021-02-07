#
# CMake bundles `FindBoost.cmake`, which is coupled to the Boost version. If
# we're on a system without a recent enough version of `FindBoost.cmake`, then
# we need to use the one bundled with Seastar.
#
# The "real" FIND_PACKAGE invocation for Boost is inside ACTOR_FIND_DEPENDENCIES.
#

# Be consistent in results from FindBoost.cmake.
# This is required because cmake-boost may return to Boost_{component}_LIBRARY:
# - /usr/lib64/libboost_foo.so
# - Boost::foo
set(Boost_NO_BOOST_CMAKE ON)

# This is the minimum version of Boost we need the CMake-bundled `FindBoost.cmake` to know about.
find_package(Boost 1.64 MODULE)

#
# Iterate through the dependency list defined below and execute `find_package`
# with the corresponding configuration for each 3rd-party dependency.
#
macro(seastar_find_dependencies)
    #
    # List of Seastar dependencies that is meant to be used
    # both in Seastar configuration and by clients which
    # consume Seastar via SeastarConfig.cmake.
    #
    set(_seastar_all_dependencies
        # Public dependencies.
        Boost
        c-ares
        cryptopp
        dpdk # No version information published.
        fmt
        lz4
        # Private and private/public dependencies.
        Concepts
        GnuTLS
        LinuxMembarrier
        Protobuf
        Sanitizers
        StdAtomic
        hwloc
        yaml-cpp)

    if(UNIX AND NOT APPLE)
        list(APPEND _seastar_all_dependencies

            lksctp-tools
            numactl
            rt)
    elseif(APPLE)

    endif()

    # Arguments to `find_package` for each 3rd-party dependency.
    # Note that the version specification is a "minimal" version requirement.

    # `unit_test_framework` is not required in the case we are building Seastar
    # without the testing library, however the component is always specified as required
    # to keep the CMake code minimalistic and easy-to-use.
    set(_seastar_dep_args_Boost
        1.64.0
        COMPONENTS
        program_options
        thread
        unit_test_framework
        REQUIRED)

    set(_seastar_dep_args_c-ares 1.13 REQUIRED)
    set(_seastar_dep_args_cryptopp 5.6.5 REQUIRED)
    set(_seastar_dep_args_fmt 5.0.0 REQUIRED)
    set(_seastar_dep_args_lz4 1.7.3 REQUIRED)
    set(_seastar_dep_args_GnuTLS 3.3.26 REQUIRED)
    set(_seastar_dep_args_Protobuf 2.5.0 REQUIRED)
    set(_seastar_dep_args_StdAtomic REQUIRED)
    set(_seastar_dep_args_hwloc 1.11.2)
    set(_seastar_dep_args_lksctp-tools REQUIRED)
    set(_seastar_dep_args_rt REQUIRED)
    set(_seastar_dep_args_yaml-cpp 0.5.1 REQUIRED)

    foreach(third_party ${_seastar_all_dependencies})
        find_package("${third_party}" ${_seastar_dep_args_${third_party}})
    endforeach()
endmacro()
