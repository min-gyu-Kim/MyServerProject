#include "GameWindow.hpp"
#include "../Base/Render/GDI/Renderer.hpp"

namespace game {

namespace {
int testTextureID;
}

GameWindow::GameWindow()
{
}

GameWindow::~GameWindow()
{
    delete mRenderer;
}

void GameWindow::OnCreate()
{
    mRenderer = new gdi::Renderer(this);
    testTextureID = mRenderer->LoadTexture("1720396797316.bmp");
}

void GameWindow::OnKeyDown(unsigned char keyCode)
{
    mInput.KeyDown(keyCode);
}

void GameWindow::OnKeyUp(unsigned char keyCode)
{
    mInput.KeyUp(keyCode);
}

void GameWindow::OnResize(const SIZE& windowSize)
{
}

void GameWindow::Update()
{
    mRenderer->Clear();
    mRenderer->RenderTexture(testTextureID, {10, 10});
    mRenderer->Swap();

    mInput.PostUpdate();
}

} // namespace game