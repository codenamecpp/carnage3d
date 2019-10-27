#include "stdafx.h"
#include "Inputs.h"

Inputs gInputs;

Inputs::Inputs()
{
    Cleanup();
}

void Inputs::HandleEvent(MouseButtonInputEvent& inputEvent)
{
    mMouseButtons[inputEvent.mButton] = inputEvent.mPressed;
}

void Inputs::HandleEvent(MouseMovedInputEvent& inputEvent)
{
    mCursorPositionX = inputEvent.mCursorPositionX;
    mCursorPositionY = inputEvent.mCursorPositionY;
}

void Inputs::HandleEvent(MouseScrollInputEvent& inputEvent)
{
}

void Inputs::HandleEvent(GamepadInputEvent& inputEvent)
{
    debug_assert(inputEvent.mGamepad < MAX_GAMEPADS);
    debug_assert(inputEvent.mButton < eGamepadButton_COUNT);
    mGamepadsState[inputEvent.mGamepad].mButtons[inputEvent.mButton] = inputEvent.mPressed;
}

void Inputs::HandleEvent(KeyInputEvent& inputEvent)
{
    mKeyboardKeys[inputEvent.mKeycode] = inputEvent.mPressed;
}

void Inputs::Cleanup()
{
    ::memset(mMouseButtons, 0, sizeof(mMouseButtons));
    ::memset(mKeyboardKeys, 0, sizeof(mKeyboardKeys));
    ::memset(mGamepadsState, 0, sizeof(mGamepadsState));
}

void Inputs::HandleEvent(KeyCharEvent& inputEvent)
{
}

void Inputs::SetGamepadPresent(int gamepad, bool isPresent)
{
    debug_assert(gamepad < MAX_GAMEPADS);

    mGamepadsState[gamepad].mPresent = isPresent;
    ::memset(mGamepadsState[gamepad].mButtons, 0, sizeof(mGamepadsState[gamepad].mButtons));
}
