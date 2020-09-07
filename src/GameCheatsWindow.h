#pragma once

#include "DebugWindow.h"

class GameCheatsWindow: public DebugWindow
{
public:
    bool mEnableMapCollisions;
    bool mEnableGravity;
    bool mEnableBlocksAnimation;
    bool mEnableDebugDraw;
    bool mEnableDrawDecorations = true;
    bool mEnableDrawObstacles = true;
    bool mEnableDrawPedestrians = true;
    bool mEnableDrawVehicles = true;
    bool mEnableDrawCityMesh = true;
    bool mEnableTrafficPedsGeneration = true;
    bool mEnableTrafficCarsGeneration = true;

public:
    GameCheatsWindow();

private:
    // process window state
    // @param deltaTime: Time since last frame
    void DoUI(ImGuiIO& imguiContext) override;

    void CreateCarNearby(VehicleInfo* carStyle, Pedestrian* pedestrian);
};

extern GameCheatsWindow gGameCheatsWindow;