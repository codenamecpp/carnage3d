#pragma once

#include "GuiDefs.h"
#include "GameDefs.h"

// In-game heads-up-display
class HUD final: public cxx::noncopyable
{
public:
    // Initialze HUD
    void Setup(Pedestrian* character);
    
    void UpdateFrame();
    void DrawFrame(GuiContext& guiContext);

    // Show messages
    void PushCarNameMessage(eVehicleModel vehicleModel);
    void PushAreaNameMessage();
    void PushPagerMessage();
    void PushHelpMessage();
    void PushMissionMessage();
    void PushBombCostMessage();

    void ClearTextMessages();

private:
    Pedestrian* mCharacter = nullptr;
    Font* mFont = nullptr;
    Font* mBigFont = nullptr;

    std::deque<HUDMessageData> mTextMessagesQueue;
};