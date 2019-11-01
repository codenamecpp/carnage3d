#pragma once

#include "DebugWindow.h"

class GameCheatsWindow: public DebugWindow
{
public:
    bool mDrawMapLayers[MAP_LAYERS_COUNT];
    bool mEnableMapCollisions;
    bool mEnableGravity;
    bool mEnableBlocksAnimation;

public:
    GameCheatsWindow();
    // process window state
    // @param deltaTime: Time since last frame
    void DoUI(Timespan deltaTime) override;
private:
};

extern GameCheatsWindow gGameCheatsWindow;