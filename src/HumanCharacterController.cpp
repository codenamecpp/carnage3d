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
        inputEvent.SetConsumed();
    }

    if (inputEvent.mKeycode == KEYCODE_RIGHT)
    {
        mCharacter->mCtlActions[ePedestrianAction_TurnRight] = inputEvent.mPressed;
        inputEvent.SetConsumed();
    }

    if (inputEvent.mKeycode == KEYCODE_UP)
    {
        mCharacter->mCtlActions[ePedestrianAction_Run] = inputEvent.mPressed;
        inputEvent.SetConsumed();
    }

    if (inputEvent.mKeycode == KEYCODE_DOWN)
    {
        mCharacter->mCtlActions[ePedestrianAction_WalkBackward] = inputEvent.mPressed;
        inputEvent.SetConsumed();
    }

    if (inputEvent.mKeycode == KEYCODE_SPACE)
    {
        mCharacter->mCtlActions[ePedestrianAction_Jump] = inputEvent.mPressed;
        inputEvent.SetConsumed();
    }

    if (inputEvent.mKeycode == KEYCODE_LEFT_CTRL)
    {
        mCharacter->mCtlActions[ePedestrianAction_Shoot] = inputEvent.mPressed;
        inputEvent.SetConsumed();
    }

    if (inputEvent.mKeycode == KEYCODE_Z && inputEvent.mPressed)
    {
        SwitchPrevWeapon();
        inputEvent.SetConsumed();
    }

    if (inputEvent.mKeycode == KEYCODE_X && inputEvent.mPressed)
    {
        SwitchNextWeapon();
        inputEvent.SetConsumed();
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

void HumanCharacterController::SwitchNextWeapon()
{
    int nextWeaponIndex = (mCharacter->mCurrentWeapon + 1) % eWeaponType_COUNT;
    for (; nextWeaponIndex != mCharacter->mCurrentWeapon; )
    {
        if (mCharacter->mWeaponsAmmo[nextWeaponIndex] != 0)
        {
            mCharacter->ChangeWeapon((eWeaponType) nextWeaponIndex);
            return;
        }
        nextWeaponIndex = (nextWeaponIndex + 1) % eWeaponType_COUNT;
    }
}

void HumanCharacterController::SwitchPrevWeapon()
{
    int nextWeaponIndex = mCharacter->mCurrentWeapon == 0 ? (eWeaponType_COUNT - 1) : (mCharacter->mCurrentWeapon - 1);
    for (; nextWeaponIndex != mCharacter->mCurrentWeapon; )
    {
        if (mCharacter->mWeaponsAmmo[nextWeaponIndex] != 0)
        {
            mCharacter->ChangeWeapon((eWeaponType) nextWeaponIndex);
            return;
        }
        nextWeaponIndex = nextWeaponIndex == 0 ? (eWeaponType_COUNT - 1) : (nextWeaponIndex - 1);
    }
}