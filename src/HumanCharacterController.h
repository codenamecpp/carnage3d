#pragma once

#include "CharacterController.h"

// defines input actions mapping for human player
class InputActionsMapping
{
public:
    InputActionsMapping() = default;

    // setup actions mapping from config node
    bool SetFromConfig(cxx::config_node& configNode);

    void SetNull();
    void SetDefaults();

public:
    eInputControllerType mControllerType = eInputControllerType_None;

    // keyboard
    std::map<eKeycode, ePedestrianAction> mKeysInCarActions;
    std::map<eKeycode, ePedestrianAction> mKeysOnFootActions;

    // gamepad
    std::map<eGamepadButton, ePedestrianAction> mGpButtonsInCarActions;
    std::map<eGamepadButton, ePedestrianAction> mGpButtonsOnFootActions;
};

//////////////////////////////////////////////////////////////////////////

class HumanCharacterController final: public CharacterController
{
public:
    // public for convenience
    InputActionsMapping mInputs;

public:
    // process controller logic
    // @param deltaTime: Time since last frame
    void UpdateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;
    void SetCharacter(Pedestrian* character);
    // process players inputs
    // @param inputEvent: Event data
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(GamepadInputEvent& inputEvent);

private:
    bool HandleInputAction(ePedestrianAction action, bool isActivated);
    void SwitchNextWeapon();
    void SwitchPrevWeapon();
    void EnterOrExitCar(bool alternative);

private:
    Pedestrian* mCharacter = nullptr;
};