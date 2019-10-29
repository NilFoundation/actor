cmake_minimum_required(VERSION 3.5)

cm_find_package(CM)
include(CMDeploy)
include(CMSetupVersion)

cm_project(cash WORKSPACE_NAME ${CMAKE_WORKSPACE_NAME} LANGUAGES CXX)

cm_setup_version(VERSION 0.16.0)

# check whether submodules are available
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/pybind/CMakeLists.txt")
    message(STATUS "Pybind submodule not loaded, skip python.")
    set(MTL_NO_PYTHON yes)
else()
    if(NOT "${MTL_PYTHON_CONFIG_BIN}" STREQUAL "")
        execute_process(COMMAND "${MTL_PYTHON_CONFIG_BIN}" --includes
                        OUTPUT_VARIABLE PYTHON_INCLUDE_FLAGS)
        string(STRIP "${PYTHON_INCLUDE_FLAGS}" PYTHON_INCLUDE_FLAGS)
        execute_process(COMMAND "${MTL_PYTHON_CONFIG_BIN}" --ldflags
                        OUTPUT_VARIABLE PYTHON_LDFLAGS)
        string(STRIP "${PYTHON_LDFLAGS}" PYTHON_LDFLAGS)
        if("${MTL_EXTRA_LDFLAGS}" STREQUAL "")
            set(MTL_EXTRA_LDFLAGS "${PYTHON_LDFLAGS}")
        else()
            set(MTL_EXTRA_LDFLAGS "${MTL_EXTRA_LDFLAGS} ${PYTHON_LDFLAGS}")
        endif()
        string(REPLACE " -I" ";-I" dir_list ${PYTHON_INCLUDE_FLAGS})
        foreach(flag ${dir_list})
            # strip -I from each path
            string(SUBSTRING "${flag}" 2 -1 dir)
            include_directories("${dir}")
        endforeach()
    else()
        cm_find_package(PythonLibs)
        if(NOT PYTHONLIBS_FOUND)
            message(STATUS "Unable to find Python, disable Python binding")
            message(STATUS "Set MTL_PYTHON_CONFIG_BIN or use './configure --with-python-config=...' to use python-conf")
            set(MTL_NO_PYTHON yes)
        else()
            message(STATUS "Found Python ${PYTHONLIBS_VERSION_STRING}")
            include_directories(${PYTHON_INCLUDE_DIRS})
        endif()
    endif()
endif()


set(MTL_PYTHON_SRCS src/main.cpp)
set(MTL_PYTHON_HDRS)

# add targets to CMake
if(NOT MTL_NO_PYTHON)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
    add_executable(mtl_python ${MTL_PYTHON_SRCS} ${MTL_PYTHON_HDRS})
    target_link_libraries(mtl_python
                          ${MTL_EXTRA_LDFLAGS}
                          ${MTL_LIBRARY_CORE}
                          ${MTL_LIBRARY_IO}
                          ${MTL_LIBRARY_RIAC}
                          ${PTHREAD_LIBRARIES}
                          ${LIBEDIT_LIBRARIES}
                          ${PYTHON_LIBRARIES})
    install(TARGETS mtl_python DESTINATION bin)
else()
    add_custom_target(mtl_python SOURCES ${MTL_PYTHON_SRCS} ${MTL_PYTHON_HDRS})
endif()

