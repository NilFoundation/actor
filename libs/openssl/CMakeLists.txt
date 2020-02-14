cmake_minimum_required(VERSION 3.5)

cm_find_package(CM)
include(CMDeploy)
include(CMSetupVersion)

cm_find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    # Check OpenSSL version >= 1.0.1
    if(OPENSSL_VERSION VERSION_LESS 1.0.1)
        message(STATUS
                "Disable OpenSSL. Required >= 1.0.1 due to TLSv1.2 support.")
        set(ACTOR_NO_OPENSSL yes)
    else()
        if(NOT CMAKE_CROSSCOMPILING)
            # Check if openssl headers and library versions match
            set(CMAKE_REQUIRED_LIBRARIES ${OPENSSL_LIBRARIES})
            set(CMAKE_REQUIRED_INCLUDES ${OPENSSL_INCLUDE_DIR})
            check_c_source_runs("
          #include <openssl/opensslv.h>
          #include <openssl/crypto.h>
          int main() {
            if (SSLeay() == OPENSSL_VERSION_NUMBER) {
              return 0;
            }
            return -1;
          }
        " OPENSSL_CORRECT_VERSION_NUMBER)
            if(NOT OPENSSL_CORRECT_VERSION_NUMBER)
                message(FATAL_ERROR
                        "OpenSSL library version does not match headers")
            endif()
        endif()
    endif()
else()
    return()
endif()

cm_project(openssl WORKSPACE_NAME ${CMAKE_WORKSPACE_NAME})

cm_setup_version(VERSION 0.16.0)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB_RECURSE ${CURRENT_PROJECT_NAME}_HEADERS "include/nil/actor/*.hpp")

# list cpp files excluding platform-dependent files
set(${CURRENT_PROJECT_NAME}_SOURCES
    src/manager.cpp
    src/middleman_actor.cpp
    src/publish.cpp
    src/remote_actor.cpp
    src/session.cpp)

add_custom_target(openssl)

if(MINGW)
    list(APPEND OPENSSL_LIBRARIES -lz)
endif()

if(NOT APPLE AND NOT WIN32)
    list(APPEND OPENSSL_LIBRARIES -lpthread)
endif()

add_library(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} ${${CURRENT_PROJECT_NAME}_HEADERS}
            ${${CURRENT_PROJECT_NAME}_SOURCES} ${OpenCL_INCLUDE_DIRS})
set_target_properties(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PROPERTIES
                      LINKER_LANGUAGE CXX
                      EXPORT_NAME ${CURRENT_PROJECT_NAME}
                      CXX_STANDARD 11
                      CXX_STANDARD_REQUIRED TRUE)

target_link_libraries(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PRIVATE
                      ${CMAKE_WORKSPACE_NAME}::actor
                      ${CMAKE_WORKSPACE_NAME}::io

                      ${OPENSSL_LIBRARIES})

target_include_directories(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME} PRIVATE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>

                           ${OPENSSL_INCLUDE_DIR})

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