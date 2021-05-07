#include "stdafx.h"
#include "CharacterController.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "DebugRenderer.h"

CharacterController::~CharacterController()
{
    DeactivateController();
}

void CharacterController::UpdateFrame()
{
    // do nothing
}

void CharacterController::DebugDraw(DebugRenderer& debugRender)
{
    // do nothing
}

bool CharacterController::IsHumanPlayer() const
{
    return false;
}

void CharacterController::DeactivateController()
{
    if (mCharacter)
    {
        debug_assert(mCharacter->mController == this);
        mCharacter->mController = nullptr;
        mCharacter->mCtlState.Clear();
        mCharacter = nullptr;
    }
}

bool CharacterController::IsControllerActive() const
{
    return mCharacter != nullptr;
}

void CharacterController::OnCharacterStartCarDrive()
{
}

void CharacterController::OnCharacterStopCarDrive()
{
}