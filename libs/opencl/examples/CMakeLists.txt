cmake_minimum_required(VERSION 3.5)

cm_find_package(CM)
include(CMDeploy)
include(CMSetupVersion)

cm_project(examples_opencl WORKSPACE_NAME ${CMAKE_WORKSPACE_NAME})

cm_setup_version(VERSION 0.16.0)

if(OpenCL_LIBRARIES AND NOT ACTOR_NO_EXAMPLES)
    add_custom_target(opencl_examples)
    include_directories(${LIBACTOR_INCLUDE_DIRS})
    if(${CMAKE_SYSTEM_NAME} MATCHES "Window")
        set(WSLIB -lws2_32)
    else()
        set(WSLIB)
    endif()
    macro(add name folder)
        add_executable(${name} ${folder}/${name}.cpp ${ARGN})
        target_link_libraries(${name}
                              ${ACTOR_EXTRA_LDFLAGS}
                              ${ACTOR_LIBRARIES}
                              ${PTHREAD_LIBRARIES}
                              ${WSLIB}
                              ${OpenCL_LIBRARIES})
        add_dependencies(${name} all_examples)
    endmacro()
    add(proper_matrix .)
    add(simple_matrix .)
    add(scan .)
endif()
