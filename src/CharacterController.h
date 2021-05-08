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

    // Whether character is under human player control
    virtual bool IsHumanPlayer() const;

    // Inactive ai controllers will be automatically destroyed on next frame
    virtual void DeactivateController();

    bool IsControllerActive() const;

    // Process character events
    virtual void OnCharacterStartCarDrive();
    virtual void OnCharacterStopCarDrive();
};