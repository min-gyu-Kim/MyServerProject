#pragma once

namespace core {
class Profiler
{
  public:
    Profiler();
    ~Profiler();

  private:
    // Implementation details
};
} // namespace core

#if defined(PROFILER_ENABLED)
#define BEGIN_PROFILE(block_name) core::Profile profile(block_name);
#define END_PROFILE() profile.Stop();
#else
#define BEGIN_PROFILE(block_name)
#define END_PROFILE()
#endif
