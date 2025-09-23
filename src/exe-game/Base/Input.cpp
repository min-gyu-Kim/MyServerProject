#include "Input.hpp"
#include <stdio.h>

namespace game {

Input::Input()
{
    for (int i = 0; i < 256; ++i) {
        mKeyStates[i] = eKeyState::NONE;
    }
}

Input::~Input()
{
}

void Input::KeyDown(unsigned char keyCode)
{
    eKeyState& keyState = mKeyStates[keyCode];
    switch (keyState) {
    case eKeyState::DOWN:
        keyState = eKeyState::PRESS;
        break;

    case eKeyState::PRESS:
        break;

    default:
        keyState = eKeyState::DOWN;
        break;
    }
}

void Input::KeyUp(unsigned char keyCode)
{
    eKeyState& keyState = mKeyStates[keyCode];
    keyState = eKeyState::UP;
}

Input::eKeyState Input::GetKeyState(unsigned char keyCode)
{
    return mKeyStates[keyCode];
}

void Input::PostUpdate()
{
    for (int i = 0; i < 256; ++i) {
        if (mKeyStates[i] == eKeyState::UP) {
            mKeyStates[i] = eKeyState::NONE;
        } else if (mKeyStates[i] == eKeyState::DOWN) {
            mKeyStates[i] = eKeyState::PRESS;
        }
    }
}

} // namespace game