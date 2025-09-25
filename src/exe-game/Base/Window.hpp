#pragma once

#include <Windows.h>

namespace game {
class Window
{
  public:
    Window();
    virtual ~Window();

    bool Create(SIZE windowSize, const char* title);

    void Resize(SIZE windowSize);

    HWND GetWindowHandle();
    SIZE GetSize() const
    {
        return mSize;
    }

  protected:
    virtual void OnCreate() = 0;
    virtual void OnResize(const SIZE& windowSize) = 0;
    virtual void OnKeyDown(unsigned char keyCode) = 0;
    virtual void OnKeyUp(unsigned char keyCode) = 0;
    virtual void Update() = 0;

  public:
    static bool Run();
    static void Stop();

  private:
    LRESULT MessageHandler(UINT message, WPARAM wParam, LPARAM lParam);

  private:
    static LRESULT __stdcall WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static void UpdateWindows();

  private:
    HWND mHandle;
    SIZE mSize;

  private:
    static bool mIsRunning;
};
} // namespace game