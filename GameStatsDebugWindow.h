#pragma once

#include "DebugWindow.h"

class GameStatsDebugWindow: public DebugWindow
{
public:
    GameStatsDebugWindow();
    // process window state
    // @param deltaTime: Time since last frame
    void DoUI(Timespan deltaTime) override;
private:
};

extern GameStatsDebugWindow gGameStatsDebugWindow;