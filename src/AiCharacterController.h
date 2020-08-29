#pragma once

#include "CharacterController.h"

// defines ai character controller
class AICharacterController final: public CharacterController
{
public:
    // process controller logic
    void UpdateFrame(Pedestrian* pedestrian) override;
};