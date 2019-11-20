#pragma once

class UiContext;

// in-game heads-up-display controller
class HUD final
{
public:
    // setup hud
    void Setup(Pedestrian* character);
    
    void UpdateFrame(Timespan deltaTile);
    void DrawFrame(UiContext& uiContext);

private:
    Pedestrian* mCharacter = nullptr;
};