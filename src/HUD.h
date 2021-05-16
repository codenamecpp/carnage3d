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
    // overridable methods
    virtual void SetupHUD();
    virtual void DrawFrame(GuiContext& guiContext);
    virtual void UpdateFrame();
    virtual void UpdatePanelSize(const Point& maxSize);
    // Set top left corner position on screen
    void SetPanelPosition(const Point& tlcornerPosition)
    {
        mPanelPosition = tlcornerPosition;
    }
    // Show or hide panel
    void ShowPanel();
    void HidePanel();
    bool IsPanelVisible() const;
protected:
    bool mIsPanelVisible = true; // whether the panel should draw and update
};

//////////////////////////////////////////////////////////////////////////
class Weapon;
class HUDWeaponPanel: public HUDPanel
{
public:
    // Setup current weapon info
    void SetWeapon(Weapon& weaponState);
    // override HUDPanel methods
    void SetupHUD() override;
    void DrawFrame(GuiContext& guiContext) override;
    void UpdatePanelSize(const Point& maxSize) override;
private:
    Sprite2D mWeaponIcon;
    Font* mAmmunitionFont = nullptr;
    int mPrevAmmunitionCount = 0;
    std::string mAmmunitionText; // cached message text
};

//////////////////////////////////////////////////////////////////////////

class HUDBigFontMessage: public HUDPanel
{
public:
    void SetMessageText(const std::string& messageText);
    // override HUDPanel methods
    void SetupHUD() override;
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
    void SetMessageText(const std::string& messageText);
    // override HUDPanel methods
    void SetupHUD() override;
    void DrawFrame(GuiContext& guiContext) override;
    void UpdatePanelSize(const Point& maxSize) override; 

private:
    Font* mMessageFont = nullptr;
    Sprite2D mBackgroundSprite;
    std::string mMessageText; // cached message text
};

//////////////////////////////////////////////////////////////////////////

class HUDDistrictNamePanel: public HUDPanel
{
public:
    void SetMessageText(const std::string& messageText);
    // override HUDPanel methods
    void SetupHUD() override;
    void DrawFrame(GuiContext& guiContext) override;
    void UpdatePanelSize(const Point& maxSize) override; 

private:
    Font* mMessageFont = nullptr;
    Sprite2D mBackgroundSpriteLeftPart;
    Sprite2D mBackgroundSpriteRightPart;
    std::string mMessageText; // cached message text
};

//////////////////////////////////////////////////////////////////////////

class HUDWantedLevelPanel: public HUDPanel
{
public:
    void SetWantedLevel(int level);
    // override HUDPanel methods
    void SetupHUD() override;
    void DrawFrame(GuiContext& guiContext) override;
    void UpdateFrame() override;
    void UpdatePanelSize(const Point& maxSize) override;
private:
    struct CopSprite: public Sprite2D
    {
        SpriteAnimation mAnimationState;
    };
    CopSprite mCopSprites[GAME_MAX_WANTED_LEVEL];
    int mCopSpritesCount = 0;
};

//////////////////////////////////////////////////////////////////////////

class HUDScoresPanel: public HUDPanel
{
public:
    void SetScores(int score, int lives, int multiplier);
    // override HUDPanel methods
    void SetupHUD() override;
    void DrawFrame(GuiContext& guiContext) override;
    void UpdateFrame() override;
    void UpdatePanelSize(const Point& maxSize) override;
private:
    Font* mFontScore = nullptr;
    Font* mFontLives = nullptr;
    Font* mFontMultiplier = nullptr;
    Point mColumnCountersDims;
    // cache score values
    int mPrevScore = 0;
    int mPrevLives = 0;
    int mPrevMultiplier = 0;
    // cache texts
    std::string mScoreText;
    std::string mLivesText;
    std::string mMultiplierText;
};

//////////////////////////////////////////////////////////////////////////

class HUDPagerPanel: public HUDPanel
{
public:

};

//////////////////////////////////////////////////////////////////////////

class HUDBonusPanel: public HUDPanel
{
public:
    // Setup current weapon info
    void SetBonus(bool showBonusKey, int armorCount);
    // override HUDPanel methods
    void SetupHUD() override;
    void DrawFrame(GuiContext& guiContext) override;
    void UpdateFrame() override;
    void UpdatePanelSize(const Point& maxSize) override;
private:
    Font* mAmmunitionFont = nullptr;
    Sprite2D mKeyIcon;
    SpriteAnimation mKeyAnimation;
    Sprite2D mArmorIcon;
    SpriteAnimation mArmorAnimation;
    bool mShowBonusKey = false;
    int mAmmunitionCount = 0;
    std::string mAmmunitionText; // cached message text
};

//////////////////////////////////////////////////////////////////////////

// In-game heads-up-display
class HUD final: public cxx::noncopyable
{
public:
    // Initialze HUD
    void SetupHUD(HumanPlayer* humanPlayer);
    
    void UpdateFrame();
    void DrawFrame(GuiContext& guiContext);

    // Show messages
    void PushPagerMessage();
    void PushHelpMessage();
    void PushMissionMessage();
    void PushBombCostMessage();

    void ShowBigFontMessage(eHUDBigFontMessage messageType);
    void ShowCarNameMessage(eVehicleModel carModel);
    // Show district name where player is located now
    void ShowDistrictNameMessage(int districtIndex);

private:

    enum eHUDPanelAlign
    {
        eHUDPanelAlign_Left,
        eHUDPanelAlign_Center,
        eHUDPanelAlign_Right
    };
    void ArrangePanels(const Rect& viewportRect);
    void ArrangePanelsHorz(const Rect& bounds, eHUDPanelAlign panelsAlign, int spacing, const std::initializer_list<HUDPanel*>& panels);
    void ArrangePanelsVert(const Rect& bounds, eHUDPanelAlign panelsAlign, int spacing, const std::initializer_list<HUDPanel*>& panels);

    void DrawArrowAboveCharacter(GuiContext& guiContext);

    // Manager auto-hide panels
    void ShowAutoHidePanel(HUDPanel* panel, float showDuration);
    void TickAutoHidePanels();

    // Test whether character is hidden by solid geometry
    bool CheckCharacterObscure() const;

private:
    HumanPlayer* mHumanPlayer = nullptr;

    std::deque<HUDMessageData> mTextMessagesQueue;

    std::vector<HUDPanel*> mPanelsList;

    // show panel for a limited time and then automatically hide it
    struct AutoHidePanel
    {
        HUDPanel* mPointer = nullptr;
        float mShowTime = 0.0f;
        float mShowDuration = 0.0f; 
    };
    std::vector<AutoHidePanel> mAutoHidePanels;

    // all hud panels
    HUDWeaponPanel mWeaponPanel;
    HUDBigFontMessage mBigFontMessage;
    HUDCarNamePanel mCarNamePanel;
    HUDDistrictNamePanel mDistrictNamePanel;
    HUDWantedLevelPanel mWantedLevelPanel;
    HUDScoresPanel mScoresPanel;
    HUDBonusPanel mBonusPanel;
};