#include "GameWindow.hpp"

namespace game {

GameWindow::GameWindow()
{
}

GameWindow::~GameWindow()
{
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

    mInput.PostUpdate();
}

} // namespace game