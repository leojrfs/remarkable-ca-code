cmake_minimum_required(VERSION 3.10)

project(HelloWorld LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall")

include(GNUInstallDirs)

add_library(hello_world_lib src/hello_world.cpp include/hello_world.h)
target_include_directories(hello_world_lib PRIVATE include)
install(TARGETS hello_world_lib
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

add_executable(hello_world src/main.cpp)
target_link_libraries(hello_world PRIVATE hello_world_lib)
target_include_directories(hello_world PRIVATE include)
install(TARGETS hello_world DESTINATION ${CMAKE_INSTALL_BINDIR})

# A simple test executable for testing the library
add_executable(test_hello_world tests/test_hello_world.cpp)
target_link_libraries(test_hello_world PRIVATE hello_world_lib)
target_include_directories(test_hello_world PRIVATE include)

if (FAILING_TEST)
    target_compile_definitions(test_hello_world PRIVATE FAIL_COMPARISON_STR="foo")
endif(FAILING_TEST)

install(TARGETS test_hello_world
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include(CTest)
add_test(NAME test_hello_world COMMAND test_hello_world)
