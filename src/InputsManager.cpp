#include "stdafx.h"
#include "InputsManager.h"
#include "ImGuiManager.h"
#include "CarnageGame.h"

InputsManager gInputs;

InputsManager::InputsManager()
{
    Cleanup();
}

void InputsManager::InputEvent(MouseButtonInputEvent& inputEvent)
{
    mMouseButtons[inputEvent.mButton] = inputEvent.mPressed;

    if (gImGuiManager.IsInitialized())
    {
        gImGuiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gUiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gCarnageGame.InputEvent(inputEvent);
    }
}

void InputsManager::InputEvent(MouseMovedInputEvent& inputEvent)
{
    mCursorPositionX = inputEvent.mCursorPositionX;
    mCursorPositionY = inputEvent.mCursorPositionY;

    if (gImGuiManager.IsInitialized())
    {
        gImGuiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gUiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gCarnageGame.InputEvent(inputEvent);
    }
}

void InputsManager::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (gImGuiManager.IsInitialized())
    {
        gImGuiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gUiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gCarnageGame.InputEvent(inputEvent);
    }
}

void InputsManager::InputEvent(GamepadInputEvent& inputEvent)
{
    debug_assert(inputEvent.mGamepad < MAX_GAMEPADS);
    debug_assert(inputEvent.mButton < eGamepadButton_COUNT);
    mGamepadsState[inputEvent.mGamepad].mButtons[inputEvent.mButton] = inputEvent.mPressed;

    if (gImGuiManager.IsInitialized())
    {
        gImGuiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gUiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gCarnageGame.InputEvent(inputEvent);
    }
}

void InputsManager::InputEvent(KeyInputEvent& inputEvent)
{
    mKeyboardKeys[inputEvent.mKeycode] = inputEvent.mPressed;

    if (gImGuiManager.IsInitialized())
    {
        gImGuiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gUiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gCarnageGame.InputEvent(inputEvent);
    }
}

void InputsManager::InputEvent(KeyCharEvent& inputEvent)
{
    if (gImGuiManager.IsInitialized())
    {
        gImGuiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gUiManager.InputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gCarnageGame.InputEvent(inputEvent);
    }
}

void InputsManager::Cleanup()
{
    ::memset(mMouseButtons, 0, sizeof(mMouseButtons));
    ::memset(mKeyboardKeys, 0, sizeof(mKeyboardKeys));
    ::memset(mGamepadsState, 0, sizeof(mGamepadsState));
}

void InputsManager::SetGamepadPresent(int gamepad, bool isPresent)
{
    debug_assert(gamepad < MAX_GAMEPADS);

    mGamepadsState[gamepad].mPresent = isPresent;
    ::memset(mGamepadsState[gamepad].mButtons, 0, sizeof(mGamepadsState[gamepad].mButtons));
}
