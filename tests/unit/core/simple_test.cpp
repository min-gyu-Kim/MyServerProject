#include <catch2/catch_test_macros.hpp>
#include <core/Debug.hpp>
#include <core/core.hpp>

void TestFunction1()
{
    core::PrintBacktrace();
}

TEST_CASE("TestFunction test", "[core]")
{
    REQUIRE(true); // Placeholder for actual test logic
    TestFunction1();
}