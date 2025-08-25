find_package(Catch2 3 CONFIG REQUIRED)

add_executable(simple_test simple_test.cpp)
target_link_libraries(simple_test PRIVATE mg::core Catch2::Catch2WithMain)
set_target_properties(simple_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

add_executable(logger_test logger_test.cpp)
target_link_libraries(logger_test PRIVATE mg::core Catch2::Catch2WithMain)
set_target_properties(logger_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

include(CTest)
include(Catch)

catch_discover_tests(simple_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
catch_discover_tests(logger_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
