#pragma once

class UiContext;

// in-game heads-up-display controller
class HUD final
{
public:
    // setup hud
    void Setup(Pedestrian* character);
    
    // draw hud
    void DrawFrame(UiContext& uiContext);

private:
    Pedestrian* mCharacter = nullptr;
};