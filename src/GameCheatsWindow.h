#pragma once

#include "DebugWindow.h"

class GameCheatsWindow: public DebugWindow
{
public:
    bool mEnableMapCollisions;
    bool mEnableGravity;
    bool mEnableBlocksAnimation;

public:
    GameCheatsWindow();

private:
    // process window state
    // @param deltaTime: Time since last frame
    void DoUI(Timespan deltaTime) override;

    void CreateCarNearby(CarStyle* carStyle, Pedestrian* pedestrian);
};

extern GameCheatsWindow gGameCheatsWindow;