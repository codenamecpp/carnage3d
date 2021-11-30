#pragma once

#include "GameDefs.h"
#include "DamageInfo.h"

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
    CharacterController(eCharacterControllerType controllerType);
    virtual ~CharacterController();

    // setup controllable character
    void StartController(Pedestrian* character);
    void StopController();
    bool IsControllerActive() const;
    bool IsControllerTypeAi() const;
    bool IsControllerTypeHuman() const;

protected:
    void SetCharacter(Pedestrian* character);

    // overridable
    virtual void OnControllerStart();
    virtual void OnControllerStop();
};