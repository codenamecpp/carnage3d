#include "stdafx.h"
#include "CharacterController.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "DebugRenderer.h"

CharacterController::~CharacterController()
{
    Deactivate();
}

void CharacterController::UpdateFrame()
{
    // do nothing
}

void CharacterController::DebugDraw(DebugRenderer& debugRender)
{
    // do nothing
}

void CharacterController::Deactivate()
{
    if (mCharacter)
    {
        debug_assert(mCharacter->mController == this);
        mCharacter->mController = nullptr;
        mCharacter = nullptr;
    }
}

bool CharacterController::IsActive() const
{
    return mCharacter != nullptr;
}