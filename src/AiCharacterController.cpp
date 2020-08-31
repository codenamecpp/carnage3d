#include "stdafx.h"
#include "AiCharacterController.h"
#include "Pedestrian.h"
#include "PhysicsComponents.h"
#include "CarnageGame.h"

AiCharacterController::AiCharacterController(Pedestrian* character)
{
    mCharacter = character;

    if (mCharacter)
    {
        debug_assert(mCharacter->mController == nullptr);
        mCharacter->mController = this;
    }
}

void AiCharacterController::UpdateFrame()
{
    debug_assert(mCharacter);

    if (mCharacter->IsStanding() || mCharacter->IsIdle())
    {
        if (mCharacter->IsStanding() || !ContinueWalkToPoint())
        {
            ChooseRandomPointToWalk();
        }
    }
}

bool AiCharacterController::ContinueWalkToPoint()
{
    glm::vec2 currentPosition2 = mCharacter->mPhysicsBody->GetPosition2();

    if (glm::distance(currentPosition2, mArrivalPoint) < 0.1f)
    {
        mCharacter->mCtlState.mWalkForward = false;
        mCharacter->mCtlState.mJump = false;
        return false;
    }

    if (mCharacter->IsBurn())
    {
        mCharacter->mCtlState.mRun = true;
    }

    mCharacter->mCtlState.mWalkForward = true; // make sure current direction is valid
    return true;
}

void AiCharacterController::ChooseRandomPointToWalk()
{
    // todo: temp implementation
    glm::vec2 currentPosition = mCharacter->mPhysicsBody->GetPosition2();

    float distanceToWalk = Convert::MapUnitsToMeters(2.0f);
    float angleToWalk = 360.0f * gCarnageGame.mGameRand.generate_float();

    mCharacter->mPhysicsBody->SetRotationAngle(cxx::angle_t::from_degrees(angleToWalk));

    mArrivalPoint = currentPosition + (mCharacter->mPhysicsBody->GetSignVector() * distanceToWalk);
    mCharacter->mCtlState.mWalkForward = true;
}
