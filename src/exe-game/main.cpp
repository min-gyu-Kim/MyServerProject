#include "Game/GameWindow.hpp"

int main()
{
    game::GameWindow window;
    window.Create({1280, 720}, "Test");

    game::GameWindow::Run();

    return 0;
}