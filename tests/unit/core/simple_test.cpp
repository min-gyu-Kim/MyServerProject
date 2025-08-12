#include <libcore/test.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("TestFunction test", "[core]") {
    core::test_function();
    REQUIRE(true); // Placeholder for actual test logic
}