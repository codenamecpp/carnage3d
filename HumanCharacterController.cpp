#include "stdafx.h"
#include "HumanCharacterController.h"
#include "Pedestrian.h"

void HumanCharacterController::UpdateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    debug_assert(pedestrian == mCharacter);
}

void HumanCharacterController::InputEvent(KeyInputEvent& inputEvent)
{
    debug_assert(mCharacter);
    if (inputEvent.mKeycode == KEYCODE_LEFT)
    {
        mCharacter->mCtlActions[ePedestrianAction_TurnLeft] = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == KEYCODE_RIGHT)
    {
        mCharacter->mCtlActions[ePedestrianAction_TurnRight] = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == KEYCODE_UP)
    {
        mCharacter->mCtlActions[ePedestrianAction_Run] = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == KEYCODE_DOWN)
    {
        mCharacter->mCtlActions[ePedestrianAction_WalkBackward] = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == KEYCODE_SPACE)
    {
        mCharacter->mCtlActions[ePedestrianAction_Jump] = inputEvent.mPressed;
    }
}

void HumanCharacterController::InputEvent(MouseButtonInputEvent& inputEvent)
{
}

void HumanCharacterController::InputEvent(MouseMovedInputEvent& inputEvent)
{
}

void HumanCharacterController::InputEvent(MouseScrollInputEvent& inputEvent)
{
}

void HumanCharacterController::InputEvent(KeyCharEvent& inputEvent)
{
}

void HumanCharacterController::SetCharacter(Pedestrian* character)
{
    if (mCharacter)
    {
        debug_assert(mCharacter->mController == this);
        mCharacter->mController = nullptr;
    }
    mCharacter = character;
    if (mCharacter)
    {
        mCharacter->mController = this;
    }
}
