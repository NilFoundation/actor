include(CMTest)

cm_test_link_libraries(${CMAKE_WORKSPACE_NAME}_opencl)

if(NOT Boost_UNIT_TEST_FRAMEWORK_FOUND)
    cm_find_package(Boost REQUIRED COMPONENTS system unit_test_framework)
endif()

if(NOT OpenCL_FOUND)
    cm_find_package(OpenCL REQUIRED)
endif()

cm_test_link_libraries(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME}

                       ${CMAKE_WORKSPACE_NAME}_core

                       ${Boost_LIBRARIES}
                       ${OpenCL_LIBRARIES})

cm_test(NAME opencl_test SOURCES opencl.cpp)

target_include_directories(opencl_test PRIVATE
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test/include>"
                           "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"

                           ${OpenCL_INCLUDE_DIRS}
                           ${Boost_INCLUDE_DIRS})


set_target_properties(opencl_test PROPERTIES
                      CXX_STANDARD 11
                      CXX_STANDARD_REQUIRED TRUE)

get_target_property(target_type Boost::unit_test_framework TYPE)
if(target_type STREQUAL "SHARED_LIB")
    target_compile_definitions(opencl_test PRIVATE BOOST_TEST_DYN_LINK)
elseif(target_type STREQUAL "STATIC_LIB")

endif()