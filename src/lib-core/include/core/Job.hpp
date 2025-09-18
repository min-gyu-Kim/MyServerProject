#pragma once

namespace core {
class IJob
{
  public:
    virtual bool Execute()
    {
    }

    virtual void Complete()
    {
    }
};
} // namespace core