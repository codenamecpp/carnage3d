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

void Inputs::HandleEvent(KeyInputEvent& inputEvent)
{
    mKeyboardKeys[inputEvent.mKeycode] = inputEvent.mPressed;
}

void Inputs::Cleanup()
{
    ::memset(mMouseButtons, 0, sizeof(mMouseButtons));
    ::memset(mKeyboardKeys, 0, sizeof(mKeyboardKeys));
}

void Inputs::HandleEvent(KeyCharEvent& inputEvent)
{
}