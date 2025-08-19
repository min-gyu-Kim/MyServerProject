cmake_minimum_required(VERSION 3.25)
project(MyServerProject LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_SOURCE_DIR};${CMAKE_PROJECT_NAME};ALL;/usr/lib/debug")

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
