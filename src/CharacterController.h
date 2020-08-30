#pragma once

#include "GameDefs.h"

// defines base character control class
class CharacterController: public cxx::noncopyable
{
public:
    virtual ~CharacterController();

    // Process controller logic
    virtual void UpdateFrame(Pedestrian* pedestrian);

    // Clear current state but keep target character bound to controller
    virtual void ResetControlState(Pedestrian* pedestrian);

    // Process character dead
    virtual void HandleCharacterDeath(Pedestrian* pedestrian);
};