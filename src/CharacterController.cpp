#include "stdafx.h"
#include "CharacterController.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "DebugRenderer.h"

CharacterController::~CharacterController()
{
    DeactivateConstroller();
}

void CharacterController::UpdateFrame()
{
    // do nothing
}

void CharacterController::DebugDraw(DebugRenderer& debugRender)
{
    // do nothing
}

void CharacterController::DeactivateConstroller()
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