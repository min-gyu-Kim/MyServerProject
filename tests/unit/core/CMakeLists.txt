find_package(Catch2 3 CONFIG REQUIRED)

add_executable(simple_test simple_test.cpp)
target_link_libraries(simple_test PRIVATE mg::core Catch2::Catch2WithMain)

include(CTest)
include(Catch)

catch_discover_tests(simple_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
