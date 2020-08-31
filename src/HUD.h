#pragma once

#include "Font.h"

// in-game heads-up-display controller
class HUD final
{
public:
    // setup hud
    void Setup(Pedestrian* character);
    
    void UpdateFrame();
    void DrawFrame(GuiContext& guiContext);

private:
    Pedestrian* mCharacter = nullptr;
    Font* mFont = nullptr;
};