#pragma once

#include "GenericGamestate.h"

// Main game
class GameplayGamestate: public GenericGamestate
{
public:
    GameplayGamestate() = default;

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
    void OnGamestateBroadcastEvent(const BroadcastEvent& broadcastEvent) override;

private:
    void OnHumanPlayerDie(int playerIndex);
    void OnHumanPlayerStartDriveCar(int playerIndex);
};