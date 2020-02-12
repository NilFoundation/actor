cmake_minimum_required(VERSION 3.5)

cm_find_package(CM)
include(CMDeploy)
include(CMSetupVersion)

cm_project(${CMAKE_WORKSPACE_NAME} tools LANGUAGES CXX)

cm_setup_version(VERSION 0.16.0)

add_custom_target(all_tools)

include_directories(${LIBACTOR_INCLUDE_DIRS})

macro(add name)
    add_executable(${name} ${name}.cpp ${ARGN})
    target_link_libraries(${name}
                          ${ACTOR_EXTRA_LDFLAGS}
                          ${ACTOR_LIBRARIES}
                          ${PTHREAD_LIBRARIES})
    install(FILES ${name}.cpp DESTINATION share/caf/tools/${folder})
    add_dependencies(${name} all_tools)
endmacro()

if(WIN32)
    message(STATUS "skip caf-run (not supported on Windows)")
else()
    add(actor-run)
endif()

add(actor-vec)
