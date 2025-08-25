#include "core/Logger.hpp"

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace core {
namespace {
const char *ToLevelName(eLogLevel level)
{
    switch (level) {
    case eLogLevel::Trace:
        return "TRACE";
    case eLogLevel::Debug:
        return "DEBUG";
    case eLogLevel::Info:
        return "INFO";
    case eLogLevel::Warn:
        return "WARN";
    case eLogLevel::Error:
        return "ERROR";
    case eLogLevel::Critical:
        return "CRITICAL";
    case eLogLevel::Off:
        return "OFF";
    }
    return "UNKNOWN";
}
} // namespace

core::String Logger::NowIso8601()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::system_clock;
    const auto NOW_TIME_POINT = system_clock::now();
    const auto CURRENT_TIME = system_clock::to_time_t(NOW_TIME_POINT);
    std::tm tmStruct{};
    localtime_r(&CURRENT_TIME, &tmStruct);
    const auto MILLIS =
        duration_cast<milliseconds>(NOW_TIME_POINT.time_since_epoch()) % 1000;
    return fmt::format("{:%Y-%m-%dT%H:%M:%S}.{:03d}", tmStruct,
                       static_cast<int>(MILLIS.count()));
}

core::String Logger::LevelToString(eLogLevel level)
{
    return ToLevelName(level);
}

Logger::Logger(core::String name)
    : mName(std::move(name)), mLevel(eLogLevel::Info)
{
}

Logger::Logger(core::String name,
               const std::vector<std::shared_ptr<ILogSink>> &sinks)
    : mName(std::move(name)), mLevel(eLogLevel::Info), mSinks(sinks)
{
}

Logger::Logger(core::String name,
               std::initializer_list<std::shared_ptr<ILogSink>> sinks)
    : mName(std::move(name)), mLevel(eLogLevel::Info), mSinks(sinks)
{
}

void Logger::SetLevel(eLogLevel level)
{
    mLevel = level;
}

eLogLevel Logger::GetLevel() const
{
    return mLevel;
}

// AddSink removed: sinks are immutable after construction

void Logger::Emit(eLogLevel level, const core::String &formattedMessage)
{
    const core::String TIMESTAMP = NowIso8601();
    const core::String LOGGER_NAME = mName;

    for (const auto &sink : mSinks) {
        sink->Write(level, TIMESTAMP, LOGGER_NAME, formattedMessage);
    }
}

// ConsoleSink
void ConsoleSink::Write(eLogLevel level, const core::String &timestampIso8601,
                        const core::String &loggerName,
                        const core::String &formattedMessage)
{
    const char *levelName = ToLevelName(level);
    const bool IS_ERROR = level >= eLogLevel::Error;
    if (loggerName.empty()) {
        if (IS_ERROR) {
            fmt::print(stderr, "[{}] [{}] {}\n", timestampIso8601, levelName,
                       formattedMessage);
        }
        else {
            fmt::print("[{}] [{}] {}\n", timestampIso8601, levelName,
                       formattedMessage);
        }
    }
    else {
        if (IS_ERROR) {
            fmt::print(stderr, "[{}] [{}] [{}] {}\n", timestampIso8601,
                       levelName, loggerName, formattedMessage);
        }
        else {
            fmt::print("[{}] [{}] [{}] {}\n", timestampIso8601, levelName,
                       loggerName, formattedMessage);
        }
    }
}

// FileSink
FileSink::FileSink(const core::String &filePath, bool append)
    : mFilePath(filePath), mAppend(append), mFileHandle(nullptr)
{
}

void FileSink::EnsureFileOpen()
{
    if (mFileHandle != nullptr) {
        return;
    }

    // Best-effort create parent directory if present
    const size_t SLASH_POS = mFilePath.find_last_of('/');
    if (SLASH_POS != core::String::npos) {
        core::String dir = mFilePath.substr(0, SLASH_POS);
        ::mkdir(dir.c_str(), 0755);
    }

    const char *mode = mAppend ? "a" : "w";
    std::FILE *file = std::fopen(mFilePath.c_str(), mode);
    mFileHandle = reinterpret_cast<void *>(file);
}

void FileSink::Write(eLogLevel level, const core::String &timestampIso8601,
                     const core::String &loggerName,
                     const core::String &formattedMessage)
{
    const char *levelName = ToLevelName(level);
    std::lock_guard<std::mutex> lock(mFileMutex);
    EnsureFileOpen();
    if (mFileHandle == nullptr) {
        return;
    }

    std::FILE *file = reinterpret_cast<std::FILE *>(mFileHandle);
    if (loggerName.empty()) {
        fmt::print(file, "[{}] [{}] {}\n", timestampIso8601, levelName,
                   formattedMessage);
    }
    else {
        fmt::print(file, "[{}] [{}] [{}] {}\n", timestampIso8601, levelName,
                   loggerName, formattedMessage);
    }
    std::fflush(file);
}

} // namespace core
