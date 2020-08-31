#pragma once

#include "CharacterController.h"

// defines ai character controller
class AiCharacterController final: public CharacterController
{
public:
    AiCharacterController(Pedestrian* character);

    // process controller logic
    void UpdateFrame() override;

private:
    bool ContinueWalkToPoint();
    void ChooseRandomPointToWalk();

private:
    glm::vec2 mArrivalPoint;
};