#include "stdafx.h"
#include "AiPedestrianBehavior.h"
#include "Pedestrian.h"

AiPedestrianBehavior::AiPedestrianBehavior(AiCharacterController* aiController)
    : mAiController(aiController)
{
    debug_assert(mAiController);
}

AiPedestrianBehavior::~AiPedestrianBehavior()
{
}

void AiPedestrianBehavior::ActivateBehavior()
{
    OnActivateBehavior();
}

void AiPedestrianBehavior::ShutdownBehavior()
{
    OnShutdownBehavior();
}

void AiPedestrianBehavior::UpdateBehavior()
{
}

void AiPedestrianBehavior::OnActivateBehavior()
{
}

void AiPedestrianBehavior::OnShutdownBehavior()
{
}