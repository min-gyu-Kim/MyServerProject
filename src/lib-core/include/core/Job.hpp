#pragma once

namespace core {
class IJob
{
  public:
    virtual bool Execute()
    {
        return true;
    }

    virtual void Complete()
    {
    }
};
} // namespace core