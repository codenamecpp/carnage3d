#pragma once

#include "GuiDefs.h"
#include "GameDefs.h"
#include "GuiScreen.h"

//////////////////////////////////////////////////////////////////////////

// Base class of all HUD elements
class HUDPanel: public cxx::noncopyable
{
    friend class HUD;

public:
    // readonly
    Point mScreenPosition;
    Point mSize; // manual compute it with UpdatePanelSize
    Point mMinSize;
    Point mMaxSize;
    Point mLocalPosition; 

    enum eHorzAlignMode { eHorzAlignMode_None, eHorzAlignMode_Left, eHorzAlignMode_Right, eHorzAlignMode_Center };
    eHorzAlignMode mHorzAlignMode = eHorzAlignMode_None;

    enum eVertAlignMode { eVertAlignMode_None, eVertAlignMode_Top, eVertAlignMode_Bottom, eVertAlignMode_Center };
    eVertAlignMode mVertAlignMode = eVertAlignMode_None;

    enum eLayoutMode { eLayoutMode_None, eLayoutMode_Vert, eLayoutMode_Horz };
    eLayoutMode mLayoutMode = eLayoutMode_None;

    HUDPanel* mParentContainer = nullptr;

public:
    HUDPanel();
    virtual ~HUDPanel();

    void SetupHUD();
    void DrawFrame(GuiContext& guiContext);
    void UpdateFrame();

    // Set top left corner position on screen
    void SetPosition(const Point& position);
    void SetSizeLimits(const Point& minSize, const Point& maxSize);

    void SetAlignMode(eHorzAlignMode horzAlignMode, eVertAlignMode vertAlignMode);
    void SetBorders(int borderL, int borderR, int borderT, int borderB);

    // set attached panels layout
    void SetLayoutMode(eLayoutMode layoutMode);

    // set attached panels spacing
    // @param innerSpacing: additional spacing between attached panels, works only if layout mode set
    void SetInnerSpacing(int innerSpacing);

    // manage attached panels
    void AttachPanel(HUDPanel* panel);
    void DetachPanel(HUDPanel* panel);
    void DetachPanels();

    // Show or hide panel
    void SetVisible(bool isVisible);
    bool IsVisible() const;

    void SetClipChildren(bool isClipChildren);
    bool IsClippingChildren() const;

protected:
    // overridable methods
    virtual void Self_ComputeSize(Point& outputSize) const;
    virtual void Self_DrawFrame(GuiContext& guiContext);
    virtual void Self_UpdateFrame();
    virtual void Self_SetupHUD();

protected:
    void SetParentContainer(HUDPanel* parentContainer);
    void ComputeSize();
    void ComputePosition();
    void ComputeOwnScreenPosition();

protected:
    bool mIsVisible = true; // whether the panel should draw and update
    bool mClipChildren = false;

    std::vector<HUDPanel*> mChildPanels; // all attached panels
    int mInnerSpacing = 0; // attached panels spacing
    int mBorderL = 0; // child area offset from left
    int mBorderR = 0; // child area offset from right
    int mBorderT = 0; // child area offset from top
    int mBorderB = 0; // child area offset from bottom
};

//////////////////////////////////////////////////////////////////////////

class HUDText: public HUDPanel
{
public:
    void SetTextFont(Font* textFont, int fontRemap);
    void SetText(const std::string& textString);
    void SetTextRemap(int fontRemap);
protected:
    // override HUDPanel
    void Self_ComputeSize(Point& outputSize) const override;
    void Self_DrawFrame(GuiContext& guiContext) override;
protected:
    Font* mTextFont = nullptr;
    std::string mText;
    int mTextPaletteIndex = 0;
};

//////////////////////////////////////////////////////////////////////////

class HUDSprite: public HUDPanel
{
public:
    Sprite2D mSprite;
    SpriteAnimation mAnimationState; // optional
protected:
    // override HUDPanel
    void Self_ComputeSize(Point& outputSize) const override;
    void Self_DrawFrame(GuiContext& guiContext) override;
    void Self_UpdateFrame() override;
};

//////////////////////////////////////////////////////////////////////////
class Weapon;
class HUDWeaponPanel: public HUDPanel
{
public:
    void SetWeapon(Weapon& weaponState);
protected:
    // override HUDPanel
    void Self_SetupHUD() override;
private:
    HUDSprite mIcon;
    HUDText mCounter;
    int mCurrAmmoAmount = 0;
};

//////////////////////////////////////////////////////////////////////////

class HUDBigFontMessage: public HUDText
{
protected:
    // override HUDPanel
    void Self_SetupHUD() override;
};

//////////////////////////////////////////////////////////////////////////

class HUDCarNamePanel: public HUDSprite
{
public:
    void SetMessageText(const std::string& messageText);
protected:
    // override HUDPanel
    void Self_SetupHUD() override;
private:
    HUDText mMessageText;
};

//////////////////////////////////////////////////////////////////////////

class HUDDistrictNamePanel: public HUDPanel
{
public:
    void SetMessageText(const std::string& messageText);
protected:
    // override HUDPanel
    void Self_SetupHUD() override;
private:
    HUDText mMessageText;
    HUDPanel mBgContainer;
    HUDSprite mBgLeftPart;
    HUDSprite mBgRightPart;
};

//////////////////////////////////////////////////////////////////////////

class HUDWantedLevelPanel: public HUDPanel
{
public:
    void SetWantedLevel(int level);
protected:
    // override HUDPanel
    void Self_SetupHUD() override;
private:
    HUDSprite mLevels[GAME_MAX_WANTED_LEVEL];
    int mCurrWantedLevel = 0;
};

//////////////////////////////////////////////////////////////////////////

class HUDScoresPanel: public HUDPanel
{
public:
    void SetScores(int score, int lives, int multiplier);
protected:
    // override HUDPanel
    void Self_SetupHUD() override;
    void Self_UpdateFrame() override;
private:
    HUDPanel mSmallInfoContainer;
    HUDText mScoreCounter;
    HUDText mLivesCounter;
    HUDText mMultiplierCounter;
    // cache score values
    int mCurrScore = 0;
    int mCurrLives = 0;
    int mCurrMultiplier = 0;
};

//////////////////////////////////////////////////////////////////////////

class HUDBonusPanel: public HUDPanel
{
public:
    void SetBonus(bool showBonusKey, int armorCount);
protected:
    // override HUDPanel
    void Self_SetupHUD() override;
private:
    HUDSprite mKeyIcon;
    HUDSprite mArmorIcon;
    HUDText mArmorCounter;
    int mCurrArmorAmount = 0;
    bool mShowBonusKey = false;
};

//////////////////////////////////////////////////////////////////////////

class HUDPagerMessage: public HUDPanel
{
public:
    void SetTextScrolling(bool isScrolling);
    bool IsTextScrolling() const;
protected:
    // override HUDPanel
    void Self_SetupHUD() override;
    void Self_UpdateFrame() override;
private:
    void UpdateTextScroll(float dt);
    void UpdatePagerFlash(float dt);
private:
    HUDSprite mBackground;
    HUDSprite mFlash;
    HUDText mMessageText;
    HUDPanel mMessageContainer;
    float mFlashTimeDelta = 0.0; // seconds since last flash state change
    float mScrollTimeDelta = 0.0; // seconds since last text scroll
    int mScrollTextValue = 0;
    bool mScrollingText = false;
};

//////////////////////////////////////////////////////////////////////////

// In-game heads-up-display
class HUD: public GuiScreen
{
public:
    // Initialze HUD
    void InitHUD(HumanPlayer* humanPlayer);
    void DeinitHUD();
    
    // Show messages
    void PushPagerMessage();
    void PushHelpMessage();
    void PushMissionMessage();
    void PushBombCostMessage();

    void ShowBigFontMessage(eHUDBigFontMessage messageType);
    void ShowCarNameMessage(eVehicleModel carModel);
    // Show district name where player is located now
    void ShowDistrictNameMessage(int districtIndex);

    // override GuiScreen
    void UpdateScreen() override;
    void DrawScreen(GuiContext& context) override;

private:
    void DrawArrowAboveCharacter(GuiContext& guiContext);

    // Manager auto-hide panels
    void ShowAutoHidePanel(HUDPanel* panel, float showDuration);
    void TickAutoHidePanels();

    // Test whether character is hidden by solid geometry
    bool CheckCharacterObscure() const;

private:
    HumanPlayer* mHumanPlayer = nullptr;

    std::deque<HUDMessageData> mTextMessagesQueue;

    HUDPanel mPanelsContainer;

    // show panel for a limited time and then automatically hide it
    struct AutoHidePanel
    {
        HUDPanel* mPointer = nullptr;
        float mShowTime = 0.0f;
        float mShowDuration = 0.0f; 
    };
    std::vector<AutoHidePanel> mAutoHidePanels;

    // all hud panels
    HUDPanel mTopLeftContainer;
    HUDPanel mTopMiddleContainer;
    HUDPanel mTopRightContainer;

    HUDWeaponPanel mWeaponPanel;
    HUDBigFontMessage mBigFontMessage;
    HUDCarNamePanel mCarNamePanel;
    HUDDistrictNamePanel mDistrictNamePanel;
    HUDWantedLevelPanel mWantedLevelPanel;
    HUDScoresPanel mScoresPanel;
    HUDBonusPanel mBonusPanel;
    HUDPagerMessage mPagerPanel;
};