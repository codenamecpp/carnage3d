#pragma once

#include "GameDefs.h"

class DebugRenderer;

// defines base character control class
class CharacterController: public cxx::noncopyable
{
public:
    virtual ~CharacterController();

    // Process controller logic
    virtual void UpdateFrame(Pedestrian* pedestrian);
    virtual void DebugDraw(DebugRenderer& debugRender);

    // Process character dead
    virtual void HandleCharacterDeath(Pedestrian* pedestrian);
};