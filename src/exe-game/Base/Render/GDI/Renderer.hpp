#pragma once

#include <stack>
#include <Windows.h>
#include <vector>
#include "Texture.hpp"

namespace game {

class Window;
namespace gdi {
class Renderer
{
  public:
    Renderer() = delete;
    Renderer(Window* window);
    ~Renderer();

    void Resize();
    int LoadTexture(const char* fileName);
    HDC GetMemDC() const
    {
        return mMemDC;
    }
    SIZE GetSize() const;

    void Clear();
    void RenderTexture(int textureID, const POINT& pos);
    void Swap();

  private:
    HDC mDC;
    HDC mMemDC;
    HBITMAP mBitmap;
    Window* mWindow;
    std::stack<int> mTextureIDCache;
    std::vector<Texture> mTextures;
};
} // namespace gdi
} // namespace game