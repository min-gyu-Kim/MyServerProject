#include <catch2/catch_test_macros.hpp>

#include <core/Logger.hpp>

#include <cstdio>
#include <ctime>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

class TestSink : public core::ILogSink
{
  public:
    struct Entry
    {
        core::eLogLevel mLevel;
        core::String mTimestamp;
        core::String mLoggerName;
        core::String mMessage;
    };

    void Write(core::eLogLevel level, const core::String& timestampIso8601,
               const core::String& loggerName, const core::String& formattedMessage) override
    {
        Entry entry{level, timestampIso8601, loggerName, formattedMessage};
        mEntries.push_back(entry);
    }

    const std::vector<Entry>& Entries() const
    {
        return mEntries;
    }

  private:
    std::vector<Entry> mEntries;
};

std::string MakeTempPath(const char* prefix)
{
    const auto currentTime = std::time(nullptr);
    char buf[256];
    std::snprintf(buf, sizeof(buf), "./tmp/%s-%ld-%d.log", prefix, static_cast<long>(currentTime),
                  static_cast<int>(::getpid()));
    return std::string(buf);
}

} // namespace

TEST_CASE("Logger level filtering and custom sink", "[logger]")
{
    core::Logger logger("Unit");
    logger.SetLevel(core::eLogLevel::Info);

    auto sink = std::make_shared<TestSink>();
    core::Logger logger2("Unit", {sink});

    logger2.Debug("debug {}", 1); // filtered out
    logger2.Info("info {}", 2);
    logger2.Error("error {}", 3);

    const auto& entries = sink->Entries();
    REQUIRE(entries.size() == 2);
    REQUIRE(entries[0].mLevel == core::eLogLevel::Info);
    REQUIRE(entries[1].mLevel == core::eLogLevel::Error);
    REQUIRE(entries[0].mLoggerName == core::String("Unit"));
}

TEST_CASE("FileSink writes formatted lines", "[logger][file]")
{
    const std::string path = MakeTempPath("logger-test");

    core::Logger logger("File", {std::make_shared<core::FileSink>(path, /*append=*/false)});
    logger.SetLevel(core::eLogLevel::Trace);

    logger.Info("hello {}", 123);
    logger.Warn("world");

    std::ifstream inStream(path.c_str());
    REQUIRE(inStream.good());
    std::string line1;
    std::getline(inStream, line1);
    std::string line2;
    std::getline(inStream, line2);
    inStream.close();

    REQUIRE((line1.find("[INFO]") != std::string::npos || line1.find("INFO") != std::string::npos));
    REQUIRE(line1.find("hello 123") != std::string::npos);
    REQUIRE((line2.find("[WARN]") != std::string::npos || line2.find("WARN") != std::string::npos));
}
