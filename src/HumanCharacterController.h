#pragma once

#include "CharacterController.h"
#include "InputActionsMapping.h"

//////////////////////////////////////////////////////////////////////////

class HumanCharacterController final: public CharacterController
{
public:
    // readonly
    InputActionsMapping mActionsMapping;

    glm::vec3 mSpawnPosition;

public:
    void SetCharacter(Pedestrian* character);

    // process controller logic
    void UpdateFrame() override;
    void DeactivateConstroller() override;

    // process players inputs
    // @param inputEvent: Event data
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(GamepadInputEvent& inputEvent);
    void InputEventLost();

private:
    void SwitchNextWeapon();
    void SwitchPrevWeapon();
    void EnterOrExitCar(bool alternative);

    void Respawn();

    void ProcessRepetitiveActions();
    void ProcessInputAction(eInputAction action, bool isActivated);
    bool GetActionState(eInputAction action) const;

private:
    float mRespawnTime;
    bool mUpdateInputs = false;
};