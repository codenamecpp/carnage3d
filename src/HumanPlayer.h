#pragma once

#include "HumanPlayerView.h"
#include "CharacterController.h"
#include "InputActionsMapping.h"

// Contains human player related information but also processes character controller logic
class HumanPlayer final: public CharacterController
{
public:
    // readonly
    HumanPlayerView mPlayerView;
    InputActionsMapping mActionsMapping;
    glm::vec3 mSpawnPosition;

public:
    HumanPlayer();
    void SetCharacter(Pedestrian* character);

    // Process controller logic
    void UpdateFrame() override;
    void DeactivateConstroller() override;
    bool IsHumanPlayer() const override;

    void OnCharacterStartCarDrive() override;

    // process players inputs
    // @param inputEvent: Event data
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(GamepadInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
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
    float mRespawnTime = 0.0f;
    bool mUpdateInputs = false;
};