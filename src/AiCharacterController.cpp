#include "stdafx.h"
#include "AiCharacterController.h"

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
    eAiPedestrianBehavior behaviorID = eAiPedestrianBehavior_Civilian;
    if (mCharacter->mPedestrianType == ePedestrianType_HareKrishnasGang)
    {
        behaviorID = eAiPedestrianBehavior_HareKrishnasGang;
    }

    mAiBehavior = new AiPedestrianBehavior(this, behaviorID);
    // todo: move to somewhere
    if (behaviorID == eAiPedestrianBehavior_HareKrishnasGang)
    {
        mAiBehavior->ChangeBehaviorBits(AiBehaviorBits_PlayerFollower, AiBehaviorBits_None);
    }
    mAiBehavior->ActivateBehavior();
}

void AiCharacterController::OnControllerStop()
{
    mAiBehavior->ShutdownBehavior();
    SafeDelete(mAiBehavior);
}