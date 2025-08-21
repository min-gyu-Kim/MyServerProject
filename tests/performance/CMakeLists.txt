add_executable(uring_test uring_test.cpp)
target_link_libraries(uring_test PRIVATE core)

find_package(fmt REQUIRED)

add_executable(test_client test_client.cpp)
target_link_libraries(test_client PRIVATE fmt::fmt core)
