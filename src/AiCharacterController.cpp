#include "stdafx.h"
#include "AiCharacterController.h"
#include "AiGangBehavior.h"

AiCharacterController::AiCharacterController()
    : CharacterController(eCharacterControllerType_Ai)
{
}

AiCharacterController::~AiCharacterController()
{
    SafeDelete(mAiBehavior);
}

void AiCharacterController::UpdateFrame()
{
    if (mAiBehavior)
    {
        mAiBehavior->UpdateBehavior();
    }

    // self detach
    if (mCharacter && mCharacter->IsDead())
    {
        StopController();
    }
}

void AiCharacterController::DebugDraw(DebugRenderer& debugRender)
{
}

void AiCharacterController::FollowPedestrian(Pedestrian* pedestrian)
{
    debug_assert(pedestrian);
    if (mAiBehavior)
    {
        mAiBehavior->SetLeader(pedestrian);
    }
}

void AiCharacterController::OnControllerStart()
{
    if ((mCharacter->mPedestrianType == ePedestrianType_Gang) || 
        (mCharacter->mPedestrianType == ePedestrianType_GangLeader))
    {
        mAiBehavior = new AiGangBehavior(this);
    }
    if (mAiBehavior == nullptr)
    {
        mAiBehavior = new AiPedestrianBehavior(this, eAiPedestrianBehavior_Civilian);
    }
    mAiBehavior->ActivateBehavior();
}

void AiCharacterController::OnControllerStop()
{
    mAiBehavior->ShutdownBehavior();
    SafeDelete(mAiBehavior);
}