#pragma once

#include "../Base/Window.hpp"
#include "../Base/Input.hpp"

namespace game {

class GameWindow : public Window
{
  public:
    GameWindow();
    virtual ~GameWindow();

    void OnKeyDown(unsigned char keyCode) override;
    void OnKeyUp(unsigned char keyCode) override;
    void OnResize(const SIZE& windowSize) override;

    void Update() override;

  private:
    Input mInput;
};

} // namespace game