cmake_minimum_required(VERSION 3.10)

project(ObservabilityD LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall")

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBCURL REQUIRED libcurl)
pkg_check_modules(JSONC REQUIRED json-c)

# Add executable
add_executable(observabilityd
    src/main.cpp
    src/sys_utils.c
    src/Report.cpp
    src/SystemInfo.cpp
)

target_include_directories(observabilityd PRIVATE
    ${LIBCURL_INCLUDE_DIRS}
    ${JSONC_INCLUDE_DIRS}
    include
)
target_link_libraries(observabilityd PRIVATE
    ${LIBCURL_LIBRARIES}
    ${JSONC_LIBRARIES}
)

if(ENABLE_SYSTEMD)
    pkg_check_modules(SYSTEMD REQUIRED libsystemd)
    target_compile_definitions(observabilityd PRIVATE USE_SYSTEMD)
    target_include_directories(observabilityd PRIVATE ${SYSTEMD_INCLUDE_DIRS})
    target_link_libraries(observabilityd PRIVATE ${SYSTEMD_LIBRARIES})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(observabilityd PRIVATE DEBUG_MODE)
endif()

# Installation rules
install(TARGETS observabilityd DESTINATION /usr/sbin)
