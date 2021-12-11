#include "stdafx.h"
#include "AiGangBehavior.h"
#include "Pedestrian.h"

AiGangBehavior::AiGangBehavior(AiCharacterController* aiController)
    : AiPedestrianBehavior(aiController, eAiPedestrianBehavior_Gang)
{
    Pedestrian* character = GetCharacter();
    if (character->mPedestrianType == ePedestrianType_GangLeader)
    {
        ChangeBehaviorBits(AiBehaviorBits_GangLeader | AiBehaviorBits_PlayerFollower, AiBehaviorBits_None);
    }
}

void AiGangBehavior::OnActivateBehavior()
{
    if (CheckBehaviorBits(AiBehaviorBits_GangLeader))
    {
        Pedestrian* character = GetCharacter();
        character->StartGameObjectSound(ePedSfxChannelIndex_Misc, eSfxSampleType_Level, SfxLevel_Gang, SfxFlags_Loop);
    }
}

void AiGangBehavior::OnShutdownBehavior()
{
    if (CheckBehaviorBits(AiBehaviorBits_GangLeader))
    {
        Pedestrian* character = GetCharacter();
        character->StopGameObjectSound(ePedSfxChannelIndex_Misc);
    }
}

void AiGangBehavior::OnUpdateBehavior()
{
    if (CheckBehaviorBits(AiBehaviorBits_GangLeader))
    {
        Pedestrian* character = GetCharacter();
        if (character->IsDead())
        {
            character->StopGameObjectSound(ePedSfxChannelIndex_Misc);
        }
    }
}

