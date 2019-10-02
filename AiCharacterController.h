#pragma once

#include "CharacterController.h"

// defines ai character controller
class AiCharacterController final: public CharacterController
{
public:
    // process controller logic
    // @param deltaTime: Time since last frame
    void UpdateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;
};