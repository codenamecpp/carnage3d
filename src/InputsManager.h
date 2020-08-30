#pragma once

#include "InputsDefs.h"

//////////////////////////////////////////////////////////////////////////
// Input State Holder
//////////////////////////////////////////////////////////////////////////

class InputsManager final: public cxx::noncopyable
{
public:
    bool mMouseButtons[eMButton_COUNT];
    bool mKeyboardKeys[eKeycode_COUNT];

    int mCursorPositionX;
    int mCursorPositionY;

    GamepadState mGamepadsState[eGamepadID_COUNT];

public:
    InputsManager();

    void UpdateFrame();

    // Process input event
    // @param inputEvent: Event data
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);
    void InputEvent(GamepadInputEvent& inputEvent);

    // Reset all keys and mouse buttons state
    void Cleanup();

    // Set current mouse position
    // @param positionx: position x
    // @param positiony: position y
    void SetMousePosition(int positionx, int positiony)
    {
        mCursorPositionX = positionx;
        mCursorPositionY = positiony;
    }
    // Set current mouse button state
    // @param button: mouse button index
    // @param buttonState: button pressed/released
    void SetMouseButtonState(eMButton button, bool state)
    {
        debug_assert(button < eMButton_COUNT && button > eMButton_null);
        mMouseButtons[button] = state;
    }
    // Set current key state
    // @param keycode: keycode
    // @param keystate: key pressed/released
    void SetKeyState(eKeycode keycode, bool state)
    {
        debug_assert(keycode < eKeycode_COUNT && keycode > eKeycode_null);
        mKeyboardKeys[keycode] = state;
    }
    // Get current key state
    // @param keycode: keycode
    bool GetKeyState(eKeycode keycode) const 
    {
        debug_assert(keycode < eKeycode_COUNT && keycode > eKeycode_null);
        return mKeyboardKeys[keycode];
    }
    bool GetGamepadButtonState(eGamepadID gpID, eGamepadButton gpButton) const
    {
        debug_assert(gpID < eGamepadID_COUNT);
        debug_assert(gpButton < eGamepadButton_COUNT);
        return mGamepadsState[gpID].mButtons[gpButton];
    }
    bool GetMouseButtonL() const { return mMouseButtons[eMButton_LEFT]; }
    bool GetMouseButtonR() const { return mMouseButtons[eMButton_RIGHT]; }
    bool GetMouseButtonM() const { return mMouseButtons[eMButton_MIDDLE]; }
    // Get current mouse button state
    // @param button: mouse button index
    bool GetMouseButton(eMButton button) const 
    {
        debug_assert(button < eMButton_COUNT && button > eMButton_null);
        return mMouseButtons[button];
    }

    void SetGamepadPresent(int gamepad, bool isPresent);

private:
    void InputEventConsumed(InputEventsHandler* handler);

private:
    InputEventsHandler* mLastInputsHandler = nullptr;
    std::vector<InputEventsHandler*> mInputHandlers;
};

extern InputsManager gInputs;