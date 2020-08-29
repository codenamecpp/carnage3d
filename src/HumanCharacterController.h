#pragma once

#include "CharacterController.h"

enum ePedActionsGroup
{
    ePedActionsGroup_Common,
    ePedActionsGroup_InCar,
    ePedActionsGroup_OnFoot,
    ePedActionsGroup_COUNT
};

// defines input actions mapping for human player
class InputActionsMapping
{
public:
    InputActionsMapping();

    // setup actions mapping from config node
    void SetFromConfig(cxx::json_document_node& configNode);

    void SetDefaults();
    void Clear();

    // get action mapped to input keycode or gamepad button
    // @returns ePedestrianAction_null if no action
    ePedestrianAction GetAction(ePedActionsGroup group, eKeycode keycode) const;
    ePedestrianAction GetAction(ePedActionsGroup group, eGamepadButton gpButton) const;

public:
    eInputControllerType mControllerType = eInputControllerType_None;

    eKeycode mKeycodes[ePedestrianAction_COUNT];
    eGamepadButton mGpButtons[ePedestrianAction_COUNT];
};

//////////////////////////////////////////////////////////////////////////

class HumanCharacterController final: public CharacterController
{
public:
    // readonly
    InputActionsMapping mInputs;

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
    bool HandleInputAction(ePedestrianAction action, bool isActivated);
    void SwitchNextWeapon();
    void SwitchPrevWeapon();
    void EnterOrExitCar(bool alternative);

    void Respawn();

private:
    float mRespawnTime;
};