#pragma once

#include <fmt/core.h>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "Container.hpp"

namespace core {
enum class eLogLevel : std::uint8_t
{
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off
};

class ILogSink
{
  public:
    ILogSink() = default;
    ILogSink(const ILogSink &) = default;
    ILogSink(ILogSink &&) = default;
    ILogSink &operator=(const ILogSink &) = default;
    ILogSink &operator=(ILogSink &&) = default;

    virtual ~ILogSink() = default;
    virtual void Write(eLogLevel level, const core::String &timestampIso8601,
                       const core::String &loggerName,
                       const core::String &formattedMessage) = 0;
};

class ConsoleSink final : public ILogSink
{
  public:
    ConsoleSink() = default;
    void Write(eLogLevel level, const core::String &timestampIso8601,
               const core::String &loggerName,
               const core::String &formattedMessage) override;
};

class FileSink final : public ILogSink
{
  public:
    explicit FileSink(const core::String &filePath, bool append = true);
    void Write(eLogLevel level, const core::String &timestampIso8601,
               const core::String &loggerName,
               const core::String &formattedMessage) override;

  private:
    void EnsureFileOpen();

  private:
    core::String mFilePath;
    bool mAppend;
    std::mutex mFileMutex;
    void *mFileHandle; // opaque; implemented in source to avoid leaking headers
};

class Logger
{
  public:
    explicit Logger(core::String name = "");
    Logger(core::String name,
           const std::vector<std::shared_ptr<ILogSink>> &sinks);
    Logger(core::String name,
           std::initializer_list<std::shared_ptr<ILogSink>> sinks);

    void SetLevel(eLogLevel level);
    eLogLevel GetLevel() const;

    // Sinks are provided at construction time; no runtime mutation allowed

    template <typename... Args>
    void Log(eLogLevel level, const char *fmtString, Args &&...args)
    {
        if (level < mLevel || mLevel == eLogLevel::Off) {
            return;
        }
        Emit(level, Format(fmtString, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void Trace(const char *fmtString, Args &&...args)
    {
        Log(eLogLevel::Trace, fmtString, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void Debug(const char *fmtString, Args &&...args)
    {
        Log(eLogLevel::Debug, fmtString, std::forward<Args>(args)...);
    }
    template <typename... Args> void Info(const char *fmtString, Args &&...args)
    {
        Log(eLogLevel::Info, fmtString, std::forward<Args>(args)...);
    }
    template <typename... Args> void Warn(const char *fmtString, Args &&...args)
    {
        Log(eLogLevel::Warn, fmtString, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void Error(const char *fmtString, Args &&...args)
    {
        Log(eLogLevel::Error, fmtString, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void Critical(const char *fmtString, Args &&...args)
    {
        Log(eLogLevel::Critical, fmtString, std::forward<Args>(args)...);
    }

  private:
    static core::String NowIso8601();
    static core::String LevelToString(eLogLevel level);

    void Emit(eLogLevel level, const core::String &formattedMessage);

    template <typename... Args>
    static core::String Format(const char *fmtString, Args &&...args)
    {
        return fmt::format(fmtString, std::forward<Args>(args)...);
    }

  private:
    core::String mName;
    eLogLevel mLevel;
    core::Vector<std::shared_ptr<ILogSink>> mSinks;
};
} // namespace core
