cmake_minimum_required(VERSION 3.25)
project(MyServerProject LANGUAGES CXX)

message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_SOURCE_DIR};${CMAKE_PROJECT_NAME};ALL;/usr/lib/debug")

option(ENABLE_SANITIZERS "Enable Address/Undefined Sanitizers" OFF)
option(NOT_USE_STD_CONTAINER "Not use Standard Containers" OFF)

include(target_set_ipo)

if(ENABLE_SANITIZERS)
    include(turn_on_sanitizers)
    turn_on_sanitizers()
endif()

if(NOT_USE_STD_CONTAINER)
    add_compile_definitions(NOT_USE_STD)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG)
endif()

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
