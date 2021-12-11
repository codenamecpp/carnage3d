#pragma once

#include "AiPedestrianBehavior.h"

class AiGangBehavior: public AiPedestrianBehavior
{
public:
    AiGangBehavior(AiCharacterController* aiController);

protected:
    // override AiPedestrianBehavior
    void OnActivateBehavior() override;
    void OnShutdownBehavior() override;
    void OnUpdateBehavior() override;
};