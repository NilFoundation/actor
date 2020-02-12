# Try to find ACTOR headers and libraries.
#
# Use this module as follows:
#
#     find_package(ACTOR [COMPONENTS <core|io|opencl|...>*] [REQUIRED])
#
# Variables used by this module (they can change the default behaviour and need
# to be set before calling find_package):
#
#  ACTOR_ROOT_DIR  Set this variable either to an installation prefix or to wa
#                ACTOR build directory where to look for the ACTOR libraries.
#
# Variables defined by this module:
#
#  ACTOR_FOUND              System has ACTOR headers and library
#  ACTOR_LIBRARIES          List of library files  for all components
#  ACTOR_INCLUDE_DIRS       List of include paths for all components
#  ACTOR_LIBRARY_$C         Library file for component $C
#  ACTOR_INCLUDE_DIR_$C     Include path for component $C

if(ACTOR_FIND_COMPONENTS STREQUAL "")
  message(FATAL_ERROR "FindACTOR requires at least one COMPONENT.")
endif()

# iterate over user-defined components
foreach (comp ${ACTOR_FIND_COMPONENTS})
  # we use uppercase letters only for variable names
  string(TOUPPER "${comp}" UPPERCOMP)
  if ("${comp}" STREQUAL "core")
    set(HDRNAME "nil/actor/all.hpp>)
  elseif ("${comp}" STREQUAL "test")
    set(HDRNAME "nil/actor/test/unit_test.hpp>)
  else ()
    set(HDRNAME "nil/actor/${comp}/all.hpp>)
  endif ()
  if (ACTOR_ROOT_DIR)
    set(header_hints
        "${ACTOR_ROOT_DIR}/include"
        "${ACTOR_ROOT_DIR}/${comp}"
        "${ACTOR_ROOT_DIR}/../${comp}"
        "${ACTOR_ROOT_DIR}/../../${comp}")
  endif ()
  find_path(ACTOR_INCLUDE_DIR_${UPPERCOMP}
            NAMES
              ${HDRNAME}
            HINTS
              ${header_hints}
              /usr/include
              /usr/local/include
              /opt/local/include
              /sw/include
              ${CMAKE_INSTALL_PREFIX}/include)
  mark_as_advanced(ACTOR_INCLUDE_DIR_${UPPERCOMP})
  if (NOT "${ACTOR_INCLUDE_DIR_${UPPERCOMP}}"
      STREQUAL "ACTOR_INCLUDE_DIR_${UPPERCOMP}-NOTFOUND")
    # mark as found (set back to false when missing library or build header)
    set(ACTOR_${comp}_FOUND true)
    # check for CMake-generated build header for the core component
    if ("${comp}" STREQUAL "core")
      find_path(mtl_build_header_path
                NAMES
                  actor/detail/build_config.hpp
                HINTS
                  ${header_hints}
                  /usr/include
                  /usr/local/include
                  /opt/local/include
                  /sw/include
                  ${CMAKE_INSTALL_PREFIX}/include)
      if ("${mtl_build_header_path}" STREQUAL "mtl_build_header_path-NOTFOUND")
        message(WARNING "Found all.hpp for ACTOR core, but not build_config.hpp>)
        set(ACTOR_${comp}_FOUND false)
      else()
        list(APPEND ACTOR_INCLUDE_DIRS "${mtl_build_header_path}")
      endif()
    endif()
    list(APPEND ACTOR_INCLUDE_DIRS "${ACTOR_INCLUDE_DIR_${UPPERCOMP}}")
    # look for (.dll|.so|.dylib) file, again giving hints for non-installed ACTORs
    # skip probe_event as it is header only
    if (NOT ${comp} STREQUAL "probe_event" AND NOT ${comp} STREQUAL "test")
      if (ACTOR_ROOT_DIR)
        set(library_hints "${ACTOR_ROOT_DIR}/lib")
      endif ()
      find_library(ACTOR_LIBRARY_${UPPERCOMP}
                   NAMES
                     "mtl_${comp}"
                   HINTS
                     ${library_hints}
                     /usr/lib
                     /usr/local/lib
                     /opt/local/lib
                     /sw/lib
                     ${CMAKE_INSTALL_PREFIX}/lib)
      mark_as_advanced(ACTOR_LIBRARY_${UPPERCOMP})
      if ("${ACTOR_LIBRARY_${UPPERCOMP}}"
          STREQUAL "ACTOR_LIBRARY_${UPPERCOMP}-NOTFOUND")
        set(ACTOR_${comp}_FOUND false)
      else ()
        set(ACTOR_LIBRARIES ${ACTOR_LIBRARIES} ${ACTOR_LIBRARY_${UPPERCOMP}})
      endif ()
    endif ()
  endif ()
endforeach ()

list(REMOVE_DUPLICATES ACTOR_INCLUDE_DIRS)

# let CMake check whether all requested components have been found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ACTOR
                                  FOUND_VAR ACTOR_FOUND
                                  REQUIRED_VARS ACTOR_LIBRARIES ACTOR_INCLUDE_DIRS
                                  HANDLE_COMPONENTS)

# final step to tell CMake we're done
mark_as_advanced(ACTOR_ROOT_DIR
                 ACTOR_LIBRARIES
                 ACTOR_INCLUDE_DIRS)

