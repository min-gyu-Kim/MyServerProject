add_executable(test_server test_server.cpp)
target_link_libraries(test_server PRIVATE core)
target_compile_options(test_server PRIVATE -pg)

find_package(fmt REQUIRED)

add_executable(test_client test_client.cpp)
target_link_libraries(test_client PRIVATE fmt::fmt core)
