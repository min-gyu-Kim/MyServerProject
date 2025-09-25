#include "Renderer.hpp"
#include "../../Window.hpp"
#include "Texture.hpp"
#include <cassert>

namespace game {
namespace gdi {

Renderer::Renderer(Window* window) : mWindow(window)
{
    assert(window != nullptr);
    mDC = GetDC(window->GetWindowHandle());
    mMemDC = CreateCompatibleDC(mDC);
    Resize();
}

Renderer::~Renderer()
{
    if (mBitmap != nullptr) {
        DeleteObject(mBitmap);
        mBitmap = nullptr;
    }

    if (mMemDC != nullptr) {
        DeleteDC(mMemDC);
        mMemDC = nullptr;
    }

    if (mWindow != nullptr) {
        ReleaseDC(mWindow->GetWindowHandle(), mDC);
        mWindow = nullptr;
    }
}

void Renderer::Resize()
{
    const SIZE windowSize = mWindow->GetSize();
    HBITMAP hBitmap = CreateCompatibleBitmap(mDC, windowSize.cx, windowSize.cy);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(mMemDC, hBitmap);
    DeleteObject(hOldBitmap);

    mBitmap = hBitmap;
}

int Renderer::LoadTexture(const char* fileName)
{
    assert(fileName != nullptr);
    HBITMAP hBitmap = (HBITMAP)LoadImageA(nullptr, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hBitmap == nullptr) {
        return -1;
    }
    HDC hDC = CreateCompatibleDC(mDC);

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);
    DeleteObject(hOldBitmap);

    BITMAP bitmapInfo{};
    GetObjectA(hBitmap, sizeof(BITMAP), &bitmapInfo);

    int textureID = -1;
    if (mTextureIDCache.empty()) {
        mTextures.emplace_back();
        textureID = (int)mTextures.size() - 1;
    } else {
        textureID = mTextureIDCache.top();
        mTextureIDCache.pop();
    }

    Texture& texture = mTextures[textureID];
    texture.mDC = hDC;
    texture.mTextureBitmap = hBitmap;
    texture.mID = textureID;
    texture.mSize.cx = bitmapInfo.bmWidth;
    texture.mSize.cy = bitmapInfo.bmHeight;

    return textureID;
}

SIZE Renderer::GetSize() const
{
    return mWindow->GetSize();
}

void Renderer::Clear()
{
    const SIZE windowSize = mWindow->GetSize();
    Rectangle(mMemDC, 0, 0, windowSize.cx, windowSize.cy);
}

void Renderer::RenderTexture(int textureID, const POINT& pos)
{
    assert(textureID != -1);
    Texture& texture = mTextures[textureID];
    assert(texture.GetID() == textureID);

    texture.Render(this, pos, false);
}

void Renderer::Swap()
{
    const SIZE windowSize = GetSize();
    BitBlt(mDC, 0, 0, windowSize.cx, windowSize.cy, mMemDC, 0, 0, SRCCOPY);
}

} // namespace gdi
} // namespace game