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

    // Get or set current police attention level
    int GetWantedLevel() const;
    void SetWantedLevel(int wantedLevel);

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

    void UpdateDistrictLocation();

private:
    int mLastDistrictIndex = 0; // district index where character was last time
    int mWantedLevel = 0; // current police attention level
    
    float mRespawnTime = 0.0f;
    bool mUpdateInputs = false;
};