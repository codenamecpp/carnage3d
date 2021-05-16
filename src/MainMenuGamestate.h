#pragma once

#include "GenericGamestate.h"

class MainMenuGamestate: public GenericGamestate
{
public:
    MainMenuGamestate() = default;

    // override GenericGamestate
    void OnGamestateEnter() override;
    void OnGamestateLeave() override;
    void OnGamestateFrame() override;
    void OnGamestateInputEvent(KeyInputEvent& inputEvent) override;
    void OnGamestateInputEvent(MouseButtonInputEvent& inputEvent) override;
    void OnGamestateInputEvent(MouseMovedInputEvent& inputEvent) override;
    void OnGamestateInputEvent(MouseScrollInputEvent& inputEvent) override;
    void OnGamestateInputEvent(KeyCharEvent& inputEvent) override;
    void OnGamestateInputEvent(GamepadInputEvent& inputEvent) override;
    void OnGamestateInputEventLost() override;

private:

};
