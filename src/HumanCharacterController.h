#pragma once

#include "CharacterController.h"

class HumanCharacterController final: public CharacterController
{
public:
    // process controller logic
    // @param deltaTime: Time since last frame
    void UpdateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;
    void SetCharacter(Pedestrian* character);
    // process players inputs
    // @param inputEvent: Event data
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);

private:
    void SwitchNextWeapon();
    void SwitchPrevWeapon();
    void EnterOrExitCar(bool alternative);

private:
    Pedestrian* mCharacter = nullptr;
};