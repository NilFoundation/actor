# Try to find MTL headers and libraries.
#
# Use this module as follows:
#
#     find_package(MTL [COMPONENTS <core|io|opencl|...>*] [REQUIRED])
#
# Variables used by this module (they can change the default behaviour and need
# to be set before calling find_package):
#
#  MTL_ROOT_DIR  Set this variable either to an installation prefix or to wa
#                MTL build directory where to look for the MTL libraries.
#
# Variables defined by this module:
#
#  MTL_FOUND              System has MTL headers and library
#  MTL_LIBRARIES          List of library files  for all components
#  MTL_INCLUDE_DIRS       List of include paths for all components
#  MTL_LIBRARY_$C         Library file for component $C
#  MTL_INCLUDE_DIR_$C     Include path for component $C

if(MTL_FIND_COMPONENTS STREQUAL "")
  message(FATAL_ERROR "FindMTL requires at least one COMPONENT.")
endif()

# iterate over user-defined components
foreach (comp ${MTL_FIND_COMPONENTS})
  # we use uppercase letters only for variable names
  string(TOUPPER "${comp}" UPPERCOMP)
  if ("${comp}" STREQUAL "core")
    set(HDRNAME "nil/mtl/all.hpp>)
  elseif ("${comp}" STREQUAL "test")
    set(HDRNAME "nil/mtl/test/unit_test.hpp>)
  else ()
    set(HDRNAME "nil/mtl/${comp}/all.hpp>)
  endif ()
  if (MTL_ROOT_DIR)
    set(header_hints
        "${MTL_ROOT_DIR}/include"
        "${MTL_ROOT_DIR}/${comp}"
        "${MTL_ROOT_DIR}/../${comp}"
        "${MTL_ROOT_DIR}/../../${comp}")
  endif ()
  find_path(MTL_INCLUDE_DIR_${UPPERCOMP}
            NAMES
              ${HDRNAME}
            HINTS
              ${header_hints}
              /usr/include
              /usr/local/include
              /opt/local/include
              /sw/include
              ${CMAKE_INSTALL_PREFIX}/include)
  mark_as_advanced(MTL_INCLUDE_DIR_${UPPERCOMP})
  if (NOT "${MTL_INCLUDE_DIR_${UPPERCOMP}}"
      STREQUAL "MTL_INCLUDE_DIR_${UPPERCOMP}-NOTFOUND")
    # mark as found (set back to false when missing library or build header)
    set(MTL_${comp}_FOUND true)
    # check for CMake-generated build header for the core component
    if ("${comp}" STREQUAL "core")
      find_path(mtl_build_header_path
                NAMES
                  mtl/detail/build_config.hpp
                HINTS
                  ${header_hints}
                  /usr/include
                  /usr/local/include
                  /opt/local/include
                  /sw/include
                  ${CMAKE_INSTALL_PREFIX}/include)
      if ("${mtl_build_header_path}" STREQUAL "mtl_build_header_path-NOTFOUND")
        message(WARNING "Found all.hpp for MTL core, but not build_config.hpp>)
        set(MTL_${comp}_FOUND false)
      else()
        list(APPEND MTL_INCLUDE_DIRS "${mtl_build_header_path}")
      endif()
    endif()
    list(APPEND MTL_INCLUDE_DIRS "${MTL_INCLUDE_DIR_${UPPERCOMP}}")
    # look for (.dll|.so|.dylib) file, again giving hints for non-installed MTLs
    # skip probe_event as it is header only
    if (NOT ${comp} STREQUAL "probe_event" AND NOT ${comp} STREQUAL "test")
      if (MTL_ROOT_DIR)
        set(library_hints "${MTL_ROOT_DIR}/lib")
      endif ()
      find_library(MTL_LIBRARY_${UPPERCOMP}
                   NAMES
                     "mtl_${comp}"
                   HINTS
                     ${library_hints}
                     /usr/lib
                     /usr/local/lib
                     /opt/local/lib
                     /sw/lib
                     ${CMAKE_INSTALL_PREFIX}/lib)
      mark_as_advanced(MTL_LIBRARY_${UPPERCOMP})
      if ("${MTL_LIBRARY_${UPPERCOMP}}"
          STREQUAL "MTL_LIBRARY_${UPPERCOMP}-NOTFOUND")
        set(MTL_${comp}_FOUND false)
      else ()
        set(MTL_LIBRARIES ${MTL_LIBRARIES} ${MTL_LIBRARY_${UPPERCOMP}})
      endif ()
    endif ()
  endif ()
endforeach ()

list(REMOVE_DUPLICATES MTL_INCLUDE_DIRS)

# let CMake check whether all requested components have been found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MTL
                                  FOUND_VAR MTL_FOUND
                                  REQUIRED_VARS MTL_LIBRARIES MTL_INCLUDE_DIRS
                                  HANDLE_COMPONENTS)

# final step to tell CMake we're done
mark_as_advanced(MTL_ROOT_DIR
                 MTL_LIBRARIES
                 MTL_INCLUDE_DIRS)

