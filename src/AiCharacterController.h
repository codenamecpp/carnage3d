#pragma once

#include "CharacterController.h"

// defines ai character controller
class AiCharacterController final: public CharacterController
{
public:
    // process controller logic
    void UpdateFrame(Pedestrian* pedestrian) override;
};