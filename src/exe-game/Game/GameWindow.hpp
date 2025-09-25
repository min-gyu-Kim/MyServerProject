#pragma once

#include "../Base/Window.hpp"
#include "../Base/Input.hpp"

namespace game {

namespace gdi {
class Renderer;
}

class GameWindow : public Window
{
  public:
    GameWindow();
    virtual ~GameWindow();

    void OnCreate() override;
    void OnKeyDown(unsigned char keyCode) override;
    void OnKeyUp(unsigned char keyCode) override;
    void OnResize(const SIZE& windowSize) override;

    void Update() override;

  private:
    Input mInput;
    gdi::Renderer* mRenderer;
};

} // namespace game