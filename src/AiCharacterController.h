#pragma once

#include "CharacterController.h"
#include "Pedestrian.h"
#include "AiPedestrianBehavior.h"

class AiCharacterController: public CharacterController
{
public:
    // readonly
    AiPedestrianBehavior* mAiBehavior = nullptr;

public:
    AiCharacterController();
    ~AiCharacterController();

    void UpdateFrame();
    void DebugDraw(DebugRenderer& debugRender);

    // objectives
    void FollowPedestrian(Pedestrian* pedestrian);

private:
    // override CharacterController
    void OnControllerStart() override;
    void OnControllerStop() override;
};