#include "Texture.hpp"
#include "Renderer.hpp"
#include <cassert>

namespace game {
namespace gdi {

Texture::Texture()
{
}

Texture::~Texture()
{
}

void Texture::Render(Renderer* renderer, const POINT& pos, bool xFlip)
{
    assert(renderer != nullptr);
    const HDC hMemDC = renderer->GetMemDC();

    int destX = pos.x;
    int destWidth = mSize.cx;
    if (xFlip == true) {
        destX += mSize.cx;
        destWidth = -destWidth;
    }

    StretchBlt(hMemDC, destX, pos.y, destWidth, mSize.cy, mDC, 0, 0, mSize.cx, mSize.cy, SRCCOPY);
}

} // namespace gdi
} // namespace game