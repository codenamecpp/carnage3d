#pragma once

#include "GameDefs.h"

// defines base character control class
class CharacterController: public cxx::noncopyable
{
public:
    virtual ~CharacterController();

    // process controller logic
    // @param deltaTime: Time since last frame
    virtual void UpdateFrame(Pedestrian* pedestrian, Timespan deltaTime);

    // clear current state but keep target character bound to controller
    virtual void ResetControlState(Pedestrian* pedestrian);
};