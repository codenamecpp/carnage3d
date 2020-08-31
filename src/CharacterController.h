#pragma once

#include "GameDefs.h"
#include "DamageInfo.h"

class DebugRenderer;

// Generic character controller class
class CharacterController: public cxx::noncopyable
{
public:
    // readonly
    Pedestrian* mCharacter = nullptr; // controllable character

public:
    CharacterController() = default;
    virtual ~CharacterController();

    // Process controller logic
    virtual void UpdateFrame();
    virtual void DebugDraw(DebugRenderer& debugRender);

    // Inactive ai controllers will be automatically destroyed on next frame
    void Deactivate();
    bool IsActive() const;
};