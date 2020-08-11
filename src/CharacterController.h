#pragma once

#include "GameDefs.h"

// defines base character control class
class CharacterController: public cxx::noncopyable
{
public:
    virtual ~CharacterController();

    // process controller logic
    virtual void UpdateFrame(Pedestrian* pedestrian);

    // clear current state but keep target character bound to controller
    virtual void ResetControlState(Pedestrian* pedestrian);

    // process pedestrian death
    virtual void HandleCharacterDeath(Pedestrian* pedestrian);
};