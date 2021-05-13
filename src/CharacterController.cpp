#include "stdafx.h"
#include "CharacterController.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "DebugRenderer.h"

CharacterController::CharacterController(Pedestrian* character)
{
    AssignCharacter(character);
}

CharacterController::~CharacterController()
{
    AssignCharacter(nullptr);
}

void CharacterController::AssignCharacter(Pedestrian* character)
{
    if (mCharacter == character)
        return;

    if (mCharacter)
    {
        debug_assert(mCharacter->mController == this);
        mCharacter->mController = nullptr;
    }
    mCharacter = character;
    if (mCharacter)
    {
        debug_assert(mCharacter->mController == nullptr);
        mCharacter->mController = this;
    }
    mCtlState.Clear();
}

void CharacterController::OnCharacterUpdateFrame()
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

void CharacterController::OnCharacterChangeState(ePedestrianState prevState, ePedestrianState newState)
{
    // do nothing
}

bool CharacterController::IsControllerActive() const
{
    return mCharacter != nullptr;
}