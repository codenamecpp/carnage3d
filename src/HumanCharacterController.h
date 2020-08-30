#pragma once

#include "CharacterController.h"
#include "InputActionsMapping.h"

//////////////////////////////////////////////////////////////////////////

class HumanCharacterController final: public CharacterController
{
public:
    // readonly
    InputActionsMapping mActionsMapping;

    Pedestrian* mCharacter = nullptr;
    glm::vec3 mSpawnPosition;

public:
    // process controller logic
    void UpdateFrame(Pedestrian* pedestrian) override;
    void SetCharacter(Pedestrian* character);

    void HandleCharacterDeath(Pedestrian* pedestrian) override;

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
    void SyncActionState(eInputAction action, bool& stateFlag) const;

private:
    float mRespawnTime;
    bool mUpdateInputs = false;
};