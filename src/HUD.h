#pragma once

#include "GuiDefs.h"
#include "GameDefs.h"

//////////////////////////////////////////////////////////////////////////

// Base class of all HUD elements
class HUDPanel: public cxx::noncopyable
{
public:
    // readonly
    Point mPanelPosition;
    Point mPanelSize; // manual compute it with UpdatePanelSize

public:
    HUDPanel();
    virtual ~HUDPanel();

    // Set top left corner position on screen
    void SetPanelPosition(const Point& tlcornerPosition);
   
    // Show or hide panel
    void ShowPanel();
    void HidePanel();
    void ShowPanelWithTimeout(float timeout);

    bool IsPanelVisible() const;

    // overridable methods
    virtual bool SetupHUD();
    virtual void DrawFrame(GuiContext& guiContext);
    virtual void UpdateFrame();
    virtual void UpdatePanelSize(const Point& maxSize);

protected:
    bool mIsPanelVisible = true; // whether the panel should draw and update
    bool mIsShownWithTimeout = false; // whether the panel shown for a limited time
    float mShowTimeStart = 0.0f;
    float mShowTimeDuration = 0.0f;
};

//////////////////////////////////////////////////////////////////////////

class HUDWeaponPanel: public HUDPanel
{
public:
    HUDWeaponPanel();
    // Setup current weapon icon
    void SetWeaponIcon(eWeaponID weaponID);
    // override HUDPanel methods
    bool SetupHUD() override;
    void DrawFrame(GuiContext& guiContext) override;
    void UpdatePanelSize(const Point& maxSize) override;
private:
    Sprite2D mWeaponIcon;
};

//////////////////////////////////////////////////////////////////////////

class HUDBigFontMessage: public HUDPanel
{
public:
    HUDBigFontMessage();
    void SetMessageText(const std::string& messageText);
    // override HUDPanel methods
    bool SetupHUD() override;
    void DrawFrame(GuiContext& guiContext) override;
    void UpdatePanelSize(const Point& maxSize) override; 
private:
    Font* mMessageFont = nullptr;
    std::string mMessageText; // cached message text
};

//////////////////////////////////////////////////////////////////////////

class HUDCarNamePanel: public HUDPanel
{
public:
    HUDCarNamePanel();
    void SetMessageText(const std::string& messageText);
    // override HUDPanel methods
    bool SetupHUD() override;
    void DrawFrame(GuiContext& guiContext) override;
    void UpdatePanelSize(const Point& maxSize) override; 

private:
    Font* mMessageFont = nullptr;
    Sprite2D mBackgroundSprite;
    std::string mMessageText; // cached message text
};

//////////////////////////////////////////////////////////////////////////

// In-game heads-up-display
class HUD final: public cxx::noncopyable
{
public:
    // Initialze HUD
    void Setup(Pedestrian* character);
    
    void UpdateFrame();
    void DrawFrame(GuiContext& guiContext);

    // Show messages
    void PushAreaNameMessage();
    void PushPagerMessage();
    void PushHelpMessage();
    void PushMissionMessage();
    void PushBombCostMessage();

    void ShowBigFontMessage(eHUDBigFontMessage messageType);
    void ShowCarNameMessage(eVehicleModel carModel);

    void ClearTextMessages();

private:
    void UpdatePanelsLayout(const Rect& viewportRect);

private:
    Pedestrian* mCharacter = nullptr;

    std::deque<HUDMessageData> mTextMessagesQueue;
    std::vector<HUDPanel*> mPanelsList;

    // hud panels
    HUDWeaponPanel mWeaponPanel;
    HUDBigFontMessage mBigFontMessage;
    HUDCarNamePanel mCarNamePanel;
};