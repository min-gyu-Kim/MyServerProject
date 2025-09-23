#pragma once

namespace game {
class Input
{
  public:
    enum class eKeyState { NONE, DOWN, PRESS, UP };

  public:
    Input();
    ~Input();

    void KeyDown(unsigned char keyCode);
    void KeyUp(unsigned char keyCode);

    void PostUpdate();

    eKeyState GetKeyState(unsigned char keyCode);

  private:
    eKeyState mKeyStates[256];
};
} // namespace game