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

enum eHUDBigFontMessage
{
    eHUDBigFontMessage_MissionComplete,
    eHUDBigFontMessage_MissionFailed,
    eHUDBigFontMessage_KillFrenzy,
    eHUDBigFontMessage_FrenzyFailed,
    eHUDBigFontMessage_ExtraLifeBonus,
    eHUDBigFontMessage_Gouranga,
    eHUDBigFontMessage_YouGotIt,
    eHUDBigFontMessage_FrenzyPassed,
    eHUDBigFontMessage_BonusLost,
    eHUDBigFontMessage_Busted,
    eHUDBigFontMessage_Wasted,
    eHUDBigFontMessage_GoGoGo,
    eHUDBigFontMessage_COUNT
};

// HUD message data
struct HUDMessageData
{
    eHUDMessageType mMessageType = eHUDMessageType_CarName;
    std::string mMessageText;
};