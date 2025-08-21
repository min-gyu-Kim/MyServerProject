#pragma once

#include <backward.hpp>
#include <fmt/core.h>

// clang-format off
#if defined(DEBUG)
#if defined(__GNUC__) || defined(__clang__)
#define DEBUG_BREAK() __builtin_debugtrap()
#elif defined(_MSVC)
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK() std::raise(SIGTRAP)
#endif

#define ASSERT(condition, message) if (!(condition)) { fmt::print(stderr, "Assertion failed: {}\nFile: {}:{}\n", message, __FILE__, __LINE__); DEBUG_BREAK(); }
#else
#define DEBUG_BREAK()
#define ASSERT(condition, message)
#endif
// clang-format on

namespace core {
void PrintBacktrace()
{
    backward::StackTrace stackTrace;
    stackTrace.load_here(32);
    backward::Printer p;
    p.object = true;
    p.color_mode = backward::ColorMode::always;
    p.address = true;
    p.print(stackTrace, stderr);
}
} // namespace core
