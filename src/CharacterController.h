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
    PedestrianCtlState mCtlState;

public:
    CharacterController(Pedestrian* character);
    virtual ~CharacterController();

    void AssignCharacter(Pedestrian* character);
    bool IsControllerActive() const;

    // Process controller logic
    virtual void DebugDraw(DebugRenderer& debugRender);

    // Whether character is under human player control
    virtual bool IsHumanPlayer() const;

    // Events
    virtual void OnCharacterUpdateFrame();
    virtual void OnCharacterChangeState(ePedestrianState prevState, ePedestrianState newState);
};