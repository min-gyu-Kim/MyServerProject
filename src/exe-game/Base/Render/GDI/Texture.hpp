#pragma once

#include <Windows.h>

namespace game {
namespace gdi {

class Renderer;

class Texture
{
  public:
    Texture();
    ~Texture();

    friend class Renderer;

    int GetID() const
    {
        return mID;
    }

    void Render(Renderer* renderer, const POINT& pos, bool xFlip = false);

  private:
    int mID;
    HDC mDC;
    HBITMAP mTextureBitmap;
    SIZE mSize;
};

} // namespace gdi
} // namespace game