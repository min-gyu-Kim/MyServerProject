#include "Window.hpp"
#include "../Define.hpp"
#include <unordered_map>
#include <cassert>

namespace game {

bool Window::mIsRunning = true;

namespace {

const char* sWINDOW_CLASS_NAME = "Window";
const char* sWINDOW_DEFAULT_TITLE = "Game!";

std::unordered_map<HWND, Window*> sWindowMap;

} // namespace

Window::Window() : mHandle((HWND)INVALID_HANDLE_VALUE)
{
}

Window::~Window()
{
}

bool Window::Create(SIZE windowSize, const char* title)
{
    WNDCLASSEXA wcex{};
    wcex.cbSize = sizeof(wcex);
    wcex.lpfnWndProc = &Window::WndProc;
    wcex.lpszClassName = sWINDOW_CLASS_NAME;
    wcex.style = CS_VREDRAW | CS_HREDRAW;

    if (0 == RegisterClassExA(&wcex)) {
        return false;
    }

    mHandle = CreateWindowA(sWINDOW_CLASS_NAME, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
                            CW_USEDEFAULT, 0, NULL, NULL, NULL, this);
    if (mHandle == NULL) {
        fprintf(stderr, "Failed CreateWindow() errorCode: %ld\n", GetLastError());
        return false;
    }

    Resize(windowSize);
    ShowWindow(mHandle, SW_SHOW);
    UpdateWindow(mHandle);

    this->OnCreate();

    return true;
}

void Window::Resize(SIZE windowSize)
{
    assert(mHandle != INVALID_HANDLE_VALUE);
    assert(windowSize.cx >= 0 && windowSize.cy >= 0);
    RECT clientRect = {0, 0, windowSize.cx, windowSize.cy};

    mSize = windowSize;

    AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);

    const INT screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const INT screenHeight = GetSystemMetrics(SM_CYSCREEN);
    const INT clientWidth = clientRect.right - clientRect.left;
    const INT clientHeight = clientRect.bottom - clientRect.top;
    const INT middlePositionX = (screenWidth - clientWidth) / 2;
    const INT middlePositionY = (screenHeight - clientHeight) / 2;

    MoveWindow(mHandle, middlePositionX, middlePositionY, clientWidth, clientHeight, FALSE);
}

HWND Window::GetWindowHandle()
{
    return mHandle;
}

void Window::UpdateWindows()
{
    std::unordered_map<HWND, Window*>::iterator itr = sWindowMap.begin();

    for (; itr != sWindowMap.end(); ++itr) {
        itr->second->Update();
    }
}

bool Window::Run()
{
    MSG msg{};
    while (mIsRunning) {
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                if (sWindowMap.empty()) {
                    Stop();
                }
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        UpdateWindows();
    }

    return true;
}

void Window::Stop()
{
    mIsRunning = false;
}

LRESULT __stdcall Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE) {
        LPCREATESTRUCT createStruct = (LPCREATESTRUCT)lParam;
        Window* window = (Window*)createStruct->lpCreateParams;

        sWindowMap.insert(std::make_pair(hWnd, window));
        return DefWindowProcA(hWnd, message, wParam, lParam);
    }

    if (message == WM_DESTROY) {
        sWindowMap.erase(hWnd);
        PostQuitMessage(0);
        return 0;
    }

    std::unordered_map<HWND, Window*>::iterator findIter = sWindowMap.find(hWnd);
    if (findIter == sWindowMap.end()) {
        return DefWindowProcA(hWnd, message, wParam, lParam);
    }

    Window* window = findIter->second;

    return window->MessageHandler(message, wParam, lParam);
}

LRESULT Window::MessageHandler(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_SYSKEYDOWN:
        FALL_THROUGH;
    case WM_KEYDOWN:
        OnKeyDown(wParam);
        break;
    case WM_SYSKEYUP:
        FALL_THROUGH;
    case WM_KEYUP:
        OnKeyDown(wParam);
        break;
    default:
        break;
    }

    return DefWindowProcA(mHandle, message, wParam, lParam);
}

} // namespace game