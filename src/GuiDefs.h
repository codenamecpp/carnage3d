#pragma once

class GameCamera2D;
class GuiContext;
class Font;

enum eHUDMessageType
{
    eHUDMessageType_CarName,
    eHUDMessageType_AreaName,
    eHUDMessageType_Pager,
    eHUDMessageType_Help,
    eHUDMessageType_Mission,
    eHUDMessageType_BombCost,
    eHUDMessageType_COUNT
};

// HUD message data
struct HUDMessageData
{
    eHUDMessageType mMessageType = eHUDMessageType_CarName;
    std::string mMessageText;
};