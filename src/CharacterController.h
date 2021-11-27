#pragma once

#include "GameDefs.h"
#include "DamageInfo.h"

class DebugRenderer;

//////////////////////////////////////////////////////////////////////////

enum eCharacterControllerType
{
    eCharacterControllerType_Null,
    eCharacterControllerType_Ai,
    eCharacterControllerType_Human
};

//////////////////////////////////////////////////////////////////////////

// Generic character controller class
class CharacterController: public cxx::noncopyable
{
public:
    // readonly
    Pedestrian* mCharacter = nullptr; // controllable character
    PedestrianCtlState mCtlState;
    eCharacterControllerType mControllerType;

public:
    CharacterController(Pedestrian* character, eCharacterControllerType controllerType);
    virtual ~CharacterController();

    void AssignCharacter(Pedestrian* character);
    bool IsControllerActive() const;

    bool IsControllerTypeAi() const { return mControllerType == eCharacterControllerType_Ai; }
    bool IsControllerTypeHuman() const { return mControllerType == eCharacterControllerType_Human; }

    // Process controller logic
    virtual void DebugDraw(DebugRenderer& debugRender);
    // Events
    virtual void OnCharacterUpdateFrame();
};