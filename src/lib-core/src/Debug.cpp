#include "core/Debug.hpp"

namespace core {

void PrintBacktrace()
{
    backward::StackTrace stackTrace;
    stackTrace.load_here(32);
    backward::Printer printer;
    printer.object = true;
    printer.color_mode = backward::ColorMode::always;
    printer.address = true;
    printer.print(stackTrace, stderr);
}
} // namespace core