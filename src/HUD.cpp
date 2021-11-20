#include "stdafx.h"
#include "HUD.h"
#include "SpriteBatch.h"
#include "GuiContext.h"
#include "Pedestrian.h"
#include "SpriteManager.h"
#include "GameMapManager.h"
#include "FontManager.h"
#include "Font.h"
#include "GameTextsManager.h"
#include "TimeManager.h"
#include "HumanPlayer.h"

//////////////////////////////////////////////////////////////////////////

HUDPanel::HUDPanel()
    : mLocalPosition()
    , mSize()
{
}

HUDPanel::~HUDPanel()
{
}

void HUDPanel::SetPosition(const Point& localPosition)
{
    mLocalPosition = localPosition;
}

void HUDPanel::SetSizeLimits(const Point& minSize, const Point& maxSize)
{
    mMaxSize = maxSize;
    debug_assert(mMaxSize.x >= 0);
    debug_assert(mMaxSize.y >= 0);

    mMinSize = minSize;
    debug_assert(mMinSize.x >= 0);
    debug_assert(mMinSize.y >= 0);

    debug_assert(mMinSize.x <= mMaxSize.x);
    debug_assert(mMinSize.y <= mMaxSize.y);
}

void HUDPanel::Self_ComputeSize(Point& outputSize) const
{
    // do nothing
}

void HUDPanel::Self_DrawFrame(GuiContext& guiContext)
{
    // do nothing
}

void HUDPanel::Self_UpdateFrame()
{
    // do nothing
}

void HUDPanel::Self_SetupHUD()
{
    // do nothing
}

void HUDPanel::SetParentContainer(HUDPanel* parentContainer)
{
    mParentContainer = parentContainer;
}

void HUDPanel::ComputeSize()
{
    Point childSize_max {0, 0};
    Point childSize_acc {0, 0};

    mSize.x = 0;
    mSize.y = 0;

    bool isFirst = true;

    for (HUDPanel* currChild: mChildPanels)
    {
        if (!currChild->IsVisible())
            continue;

        currChild->ComputeSize();
        childSize_max.x = std::max(currChild->mSize.x, childSize_max.x);
        childSize_max.y = std::max(currChild->mSize.y, childSize_max.y);
        childSize_acc.x += currChild->mSize.x;
        childSize_acc.y += currChild->mSize.y;

        if (mLayoutMode == eLayoutMode_None)
        {
            if (currChild->mHorzAlignMode == eHorzAlignMode_None)
            {
                mSize.x = std::max(mSize.x, currChild->mLocalPosition.x + currChild->mSize.x);
            }
            else
            {
                mSize.x = std::max(mSize.x, currChild->mSize.x);
            }

            if (currChild->mVertAlignMode == eVertAlignMode_None)
            {
                mSize.y = std::max(mSize.y, currChild->mLocalPosition.y + currChild->mSize.y);
            }
            else
            {
                mSize.y = std::max(mSize.y, currChild->mSize.y);
            }
        }

        // spacing
        if (isFirst)
        {
            childSize_acc.x += mInnerSpacing;
            childSize_acc.y += mInnerSpacing;
        }
        isFirst = false;
    }

    Self_ComputeSize(mSize);

    if (mLayoutMode == eLayoutMode_Vert)
    {
        mSize.x = std::max(childSize_max.x, mSize.x);
        mSize.y = std::max(childSize_acc.y, mSize.y);
    }
    else if (mLayoutMode == eLayoutMode_Horz)
    {
        mSize.x = std::max(childSize_acc.x, mSize.x);
        mSize.y = std::max(childSize_max.y, mSize.y);
    }

    if (mMaxSize.x > 0)
    {
        mSize.x = std::min(mSize.x, mMaxSize.x);
    }

    if (mMaxSize.y > 0)
    {
        mSize.y = std::min(mSize.y, mMaxSize.y);
    }

    if (mMinSize.x > 0)
    {
        mSize.x = std::max(mSize.x, mMinSize.x);
    }

    if (mMinSize.y > 0)
    {
        mSize.y = std::max(mSize.y, mMinSize.y);
    }
}

void HUDPanel::ComputePosition()
{
    ComputeOwnScreenPosition();

    Point child_offet = mScreenPosition;
    for (HUDPanel* currChild: mChildPanels)
    {
        if (!currChild->IsVisible())
            continue;

        // apply layout before compute full position
        if (mLayoutMode == eLayoutMode_Horz)
        {
            currChild->mScreenPosition.x = child_offet.x;
        }
        if (mLayoutMode == eLayoutMode_Vert)
        {
            currChild->mScreenPosition.y = child_offet.y;
        }
        currChild->ComputePosition();
        child_offet.x += currChild->mSize.x + mInnerSpacing;
        child_offet.y += currChild->mSize.y + mInnerSpacing;
    }
}

void HUDPanel::ComputeOwnScreenPosition()
{
    if (mParentContainer == nullptr)
    {
        mScreenPosition = mLocalPosition;
        return;
    }
    
    if (mParentContainer->mLayoutMode != eLayoutMode_Horz)
    {
        mScreenPosition.x = mParentContainer->mScreenPosition.x + mLocalPosition.x; // compute x coord
        switch (mHorzAlignMode)
        {
        case HUDPanel::eHorzAlignMode_Right: 
            mScreenPosition.x = mParentContainer->mScreenPosition.x + (mParentContainer->mSize.x - mSize.x);
        break;
        case HUDPanel::eHorzAlignMode_Center: 
            mScreenPosition.x = mParentContainer->mScreenPosition.x + (mParentContainer->mSize.x / 2) - (mSize.x / 2);
        break;
        }
    }

    if (mParentContainer->mLayoutMode != eLayoutMode_Vert)
    {
        mScreenPosition.y = mParentContainer->mScreenPosition.y + mLocalPosition.y; // compute y coord
        switch (mVertAlignMode)
        {
        case HUDPanel::eVertAlignMode_Bottom:
            mScreenPosition.y = mParentContainer->mScreenPosition.y + (mParentContainer->mSize.y - mSize.y);
        break;
        case HUDPanel::eVertAlignMode_Center:
            mScreenPosition.y = mParentContainer->mScreenPosition.y + (mParentContainer->mSize.y / 2) - (mSize.y / 2);
        break;
        }
    }
}

void HUDPanel::SetAlignMode(eHorzAlignMode horzAlignMode, eVertAlignMode vertAlignMode)
{
    mHorzAlignMode = horzAlignMode;
    mVertAlignMode = vertAlignMode;
}

void HUDPanel::SetBorders(int borderL, int borderR, int borderT, int borderB)
{
    mBorderL = borderL;
    mBorderR = borderR;
    mBorderT = borderT;
    mBorderB = borderB;
}

void HUDPanel::SetVisible(bool isVisible)
{
    mIsVisible = isVisible;
}

bool HUDPanel::IsVisible() const
{
    return mIsVisible;
}

void HUDPanel::SetLayoutMode(eLayoutMode layoutMode)
{
    mLayoutMode = layoutMode;
}

void HUDPanel::SetInnerSpacing(int panelsSpacing)
{
    mInnerSpacing = panelsSpacing;
}

void HUDPanel::AttachPanel(HUDPanel* panel)
{
    debug_assert(panel);
    if ((panel == nullptr) || (panel == this))
        return;

    if (cxx::contains(mChildPanels, panel))
        return;

    if (panel->mParentContainer)
    {
        panel->mParentContainer->DetachPanel(panel);
    }
    panel->mParentContainer = this;
    mChildPanels.push_back(panel);
}

void HUDPanel::DetachPanel(HUDPanel* panel)
{
    debug_assert(panel);
    debug_assert(panel->mParentContainer == this);
    if (panel->mParentContainer == this)
    {
        panel->mParentContainer = nullptr;
        cxx::erase_elements(mChildPanels, panel);
    }
}

void HUDPanel::DetachPanels()
{
    for (HUDPanel* currPanel: mChildPanels)
    {
        debug_assert(currPanel->mParentContainer == this);
        currPanel->mParentContainer = nullptr;
    }
    mChildPanels.clear();
}

void HUDPanel::SetupHUD()
{
    for (HUDPanel* currPanel: mChildPanels)
    {
        currPanel->SetupHUD();
    }
    Self_SetupHUD();
}

void HUDPanel::DrawFrame(GuiContext& guiContext)
{
    Self_DrawFrame(guiContext);
    for (HUDPanel* currPanel: mChildPanels)
    {
        if (currPanel->IsVisible())
        {
            currPanel->DrawFrame(guiContext);
        }
    }
}

void HUDPanel::UpdateFrame()
{
    Self_UpdateFrame();
    for (HUDPanel* currPanel: mChildPanels)
    {
        if (currPanel->IsVisible())
        {
            currPanel->UpdateFrame();
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void HUDText::SetTextFont(Font* textFont, int fontRemap)
{
    mTextFont = textFont;
    mTextPaletteIndex = gGameMap.mStyleData.GetFontPaletteIndex(fontRemap);
}

void HUDText::SetText(const std::string& textString)
{
    mText = textString;
}

void HUDText::SetTextRemap(int fontRemap)
{
    mTextPaletteIndex = gGameMap.mStyleData.GetFontPaletteIndex(fontRemap);
}

void HUDText::Self_DrawFrame(GuiContext& guiContext)
{
    if ((mTextFont == nullptr) || mText.empty())
        return;

    mTextFont->DrawString(guiContext, mText, mScreenPosition, mSize, mTextPaletteIndex);
}

void HUDText::Self_ComputeSize(Point& outputSize) const
{
    if ((mTextFont == nullptr) || mText.empty())
        return;

    mTextFont->MeasureString(mText, mMaxSize, outputSize);
}

//////////////////////////////////////////////////////////////////////////

void HUDSprite::Self_ComputeSize(Point& outputSize) const
{
    outputSize.x = mSprite.mTextureRegion.mRectangle.w;
    outputSize.y = mSprite.mTextureRegion.mRectangle.h;
}

void HUDSprite::Self_DrawFrame(GuiContext& guiContext)
{
    mSprite.mPosition.x = mScreenPosition.x * 1.0f;
    mSprite.mPosition.y = mScreenPosition.y * 1.0f;
    guiContext.mSpriteBatch.DrawSprite(mSprite);
}

void HUDSprite::Self_UpdateFrame()
{
    if (mAnimationState.UpdateFrame(gTimeManager.mUiFrameDelta))
    {
        int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, mAnimationState.GetSpriteIndex());
        gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mSprite);
    }
}

//////////////////////////////////////////////////////////////////////////

void HUDWeaponPanel::SetWeapon(Weapon& weaponState)
{
    if ((mCurrAmmoAmount == 0) || (mCurrAmmoAmount != weaponState.mAmmunition))
    {
        mCurrAmmoAmount = weaponState.mAmmunition;
        mCounter.SetText(cxx::va("%02d", weaponState.mAmmunition));
    }

    WeaponInfo* weaponInfo = weaponState.GetWeaponInfo();
    int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, weaponInfo->mSpriteIndex);

    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mIcon.mSprite);
}

void HUDWeaponPanel::Self_SetupHUD()
{
    mIcon.mSprite.mHeight = 0.0f;
    mIcon.mSprite.mScale = HUD_SPRITE_SCALE;
    mIcon.mSprite.mOriginMode = eSpriteOrigin_TopLeft;

    Font* counterFont = gFontManager.GetFont("SUB1.FON");
    debug_assert(counterFont);
    mCounter.SetTextFont(counterFont, FontRemap_Default);

    AttachPanel(&mIcon);
    AttachPanel(&mCounter);
    mCounter.SetAlignMode(HUDPanel::eHorzAlignMode_Right, HUDPanel::eVertAlignMode_Bottom);
}

//////////////////////////////////////////////////////////////////////////

void HUDBigFontMessage::Self_SetupHUD()
{
    SetAlignMode(HUDPanel::eHorzAlignMode_Center, HUDPanel::eVertAlignMode_Center);

    Font* bigFont = gFontManager.GetFont("BIG2.FON");
    debug_assert(bigFont);

    SetTextFont(bigFont, FontRemap_Default);
}

//////////////////////////////////////////////////////////////////////////

void HUDCarNamePanel::SetMessageText(const std::string& messageText)
{
    mMessageText.SetText(messageText);
}

void HUDCarNamePanel::Self_SetupHUD()
{
    SetAlignMode(HUDPanel::eHorzAlignMode_Center, HUDPanel::eVertAlignMode_None);

    Font* messageFont = gFontManager.GetFont("SUB2.FON");
    debug_assert(messageFont);
    mMessageText.SetTextFont(messageFont, FontRemap_Default);

    int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, eSpriteID_Arrow_VehicleDisplay);
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mSprite);

    mSprite.mHeight = 0.0f;
    mSprite.mScale = HUD_SPRITE_SCALE;
    mSprite.mOriginMode = eSpriteOrigin_TopLeft;

    AttachPanel(&mMessageText);
    mMessageText.SetAlignMode(HUDPanel::eHorzAlignMode_Center, HUDPanel::eVertAlignMode_Center);
}

//////////////////////////////////////////////////////////////////////////

void HUDDistrictNamePanel::SetMessageText(const std::string& messageText)
{
    mMessageText.SetText(messageText);
}

void HUDDistrictNamePanel::Self_SetupHUD()
{
    SetAlignMode(HUDPanel::eHorzAlignMode_Center, HUDPanel::eVertAlignMode_None);

    mBgContainer.SetLayoutMode(HUDPanel::eLayoutMode_Horz);
    mBgContainer.AttachPanel(&mBgLeftPart);
    mBgContainer.AttachPanel(&mBgRightPart);
    AttachPanel(&mBgContainer);

    Font* messageFont = gFontManager.GetFont("SUB2.FON");
    debug_assert(messageFont);
    mMessageText.SetTextFont(messageFont, FontRemap_Default);
    AttachPanel(&mMessageText);
    mMessageText.SetAlignMode(HUDPanel::eHorzAlignMode_Center, HUDPanel::eVertAlignMode_Center);

    // setup left part sprite
    int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, eSpriteID_Arrow_AreaDisplayLeft);
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mBgLeftPart.mSprite);
    mBgLeftPart.mSprite.mHeight = 0.0f;
    mBgLeftPart.mSprite.mScale = HUD_SPRITE_SCALE;
    mBgLeftPart.mSprite.mOriginMode = eSpriteOrigin_TopLeft;
    // setup right part sprite
    spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, eSpriteID_Arrow_AreaDisplayRight);
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mBgRightPart.mSprite);
    mBgRightPart.mSprite.mHeight = 0.0f;
    mBgRightPart.mSprite.mScale = HUD_SPRITE_SCALE;
    mBgRightPart.mSprite.mOriginMode = eSpriteOrigin_TopLeft;
}

//////////////////////////////////////////////////////////////////////////

void HUDWantedLevelPanel::SetWantedLevel(int wantedLevel)
{
    debug_assert(mCurrWantedLevel <= GAME_MAX_WANTED_LEVEL);
    if (mCurrWantedLevel == wantedLevel)
        return;

    mCurrWantedLevel = wantedLevel;
    for (HUDSprite& currLevel: mLevels)
    {
        currLevel.SetVisible(wantedLevel-- > 0);
    }
}

void HUDWantedLevelPanel::Self_SetupHUD()
{
    SetLayoutMode(HUDPanel::eLayoutMode_Horz);
    SetAlignMode(HUDPanel::eHorzAlignMode_Center, HUDPanel::eVertAlignMode_None);

    for (HUDSprite& currLevel: mLevels)
    {
        currLevel.mSprite.mHeight = 0.0f;
        currLevel.mSprite.mScale = HUD_SPRITE_SCALE;
        currLevel.mSprite.mOriginMode = eSpriteOrigin_TopLeft;

        currLevel.mAnimationState.Clear();
        currLevel.mAnimationState.mAnimDesc.SetFrames(
            {
                eSpriteID_Arrow_WantedFrame1,
                eSpriteID_Arrow_WantedFrame2
            });
        currLevel.mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
        currLevel.SetVisible(false);
        // initial sprite
        gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, eSpriteID_Arrow_WantedFrame1, 0, currLevel.mSprite);
        AttachPanel(&currLevel);
    }
}

//////////////////////////////////////////////////////////////////////////

void HUDScoresPanel::SetScores(int score, int lives, int multiplier)
{
    if (score != mCurrScore)
    {
        mCurrScore = score;
        mScoreCounter.SetText(cxx::va("%d", score));
    }

    if (lives != mCurrLives)
    {
        mCurrLives = lives;
        mLivesCounter.SetText(cxx::va(":%d", lives));
    }

    if (multiplier != mCurrMultiplier)
    {
        mCurrMultiplier = multiplier;
        mMultiplierCounter.SetText(cxx::va(":%d", multiplier));
    }
}

void HUDScoresPanel::Self_SetupHUD()
{
    Font* fontLives = gFontManager.GetFont("MISSMUL2.FON");
    debug_assert(fontLives);
    if (fontLives)
    {
        fontLives->SetFontBaseCharCode('0');
    }
    mLivesCounter.SetTextFont(fontLives, FontRemap_Green);
    mSmallInfoContainer.AttachPanel(&mLivesCounter);

    Font* fontMultiplier = gFontManager.GetFont("MISSMUL2.FON");
    debug_assert(fontMultiplier);
    if (fontMultiplier)
    {
        fontMultiplier->SetFontBaseCharCode('0');
    }
    mMultiplierCounter.SetTextFont(fontMultiplier, FontRemap_Default);
    mSmallInfoContainer.AttachPanel(&mMultiplierCounter);

    mSmallInfoContainer.SetLayoutMode(HUDPanel::eLayoutMode_Vert);
    mSmallInfoContainer.SetAlignMode(HUDPanel::eHorzAlignMode_Left, HUDPanel::eVertAlignMode_Top);
    AttachPanel(&mSmallInfoContainer);

    Font* fontScore = gFontManager.GetFont("SCORE2.FON");
    debug_assert(fontScore);
    if (fontScore)
    {
        fontScore->SetFontBaseCharCode('0');
    }
    mScoreCounter.SetTextFont(fontScore, FontRemap_Default);
    AttachPanel(&mScoreCounter);
    SetLayoutMode(HUDPanel::eLayoutMode_Horz);
}

void HUDScoresPanel::Self_UpdateFrame()
{
    // todo: implement scrolling effect for score digits
}

//////////////////////////////////////////////////////////////////////////

void HUDBonusPanel::SetBonus(bool showBonusKey, int armorCount)
{
    if ((mCurrArmorAmount == 0) || (mCurrArmorAmount != armorCount))
    {
        mArmorCounter.SetText(cxx::va("%d", armorCount));
        mCurrArmorAmount = armorCount;
    }

    mShowBonusKey = showBonusKey;
}

void HUDBonusPanel::Self_SetupHUD()
{
    SetLayoutMode(HUDPanel::eLayoutMode_Horz);

    mKeyIcon.mSprite.mHeight = 0.0f;
    mKeyIcon.mSprite.mScale = HUD_SPRITE_SCALE;
    mKeyIcon.mSprite.mOriginMode = eSpriteOrigin_TopLeft;

    mKeyIcon.mAnimationState.Clear();
    mKeyIcon.mAnimationState.mAnimDesc.SetFrames(
        {
            eSpriteID_Arrow_BonusKeyFrame1,
            eSpriteID_Arrow_BonusKeyFrame2
        });
    mKeyIcon.mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
    AttachPanel(&mKeyIcon);

    // setup sprite
    int keySpriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, mKeyIcon.mAnimationState.GetSpriteIndex());
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, keySpriteIndex, 0, mKeyIcon.mSprite);

    mArmorIcon.mSprite.mHeight = 0.0f;
    mArmorIcon.mSprite.mScale = HUD_SPRITE_SCALE;
    mArmorIcon.mSprite.mOriginMode = eSpriteOrigin_TopLeft;

    mArmorIcon.mAnimationState.Clear();
    mArmorIcon.mAnimationState.mAnimDesc.SetFrames(
        {
            eSpriteID_Arrow_BonusArmorFrame1,
            eSpriteID_Arrow_BonusArmorFrame2
        });
    mArmorIcon.mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
    AttachPanel(&mArmorIcon);

    // setup sprite
    int armorSpriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, mArmorIcon.mAnimationState.GetSpriteIndex());
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, armorSpriteIndex, 0, mArmorIcon.mSprite);

    Font* font = gFontManager.GetFont("SUB1.FON");
    debug_assert(font);
    mArmorCounter.SetTextFont(font, FontRemap_Red);
    mArmorCounter.SetAlignMode(HUDPanel::eHorzAlignMode_Right, HUDPanel::eVertAlignMode_Bottom);
    mArmorIcon.AttachPanel(&mArmorCounter);
}

//////////////////////////////////////////////////////////////////////////

void HUD::SetupHUD(HumanPlayer* humanPlayer)
{
    mHumanPlayer = humanPlayer;
    debug_assert(mHumanPlayer);

    mAutoHidePanels.clear();
    mTextMessagesQueue.clear();

    // setup hud panels
    mTopLeftContainer.SetAlignMode(HUDPanel::eHorzAlignMode_Left, HUDPanel::eVertAlignMode_Top);
    mTopLeftContainer.SetLayoutMode(HUDPanel::eLayoutMode_Vert);
    mTopLeftContainer.AttachPanel(&mWeaponPanel);
    mTopLeftContainer.AttachPanel(&mBonusPanel);

    mTopMiddleContainer.SetAlignMode(HUDPanel::eHorzAlignMode_Center, HUDPanel::eVertAlignMode_Top);
    mTopMiddleContainer.SetLayoutMode(HUDPanel::eLayoutMode_Vert);
    mTopMiddleContainer.SetInnerSpacing(5);
    mTopMiddleContainer.AttachPanel(&mWantedLevelPanel);
    mTopMiddleContainer.AttachPanel(&mDistrictNamePanel);
    mTopMiddleContainer.AttachPanel(&mCarNamePanel);

    mTopRightContainer.SetAlignMode(HUDPanel::eHorzAlignMode_Right, HUDPanel::eVertAlignMode_Top);
    mTopRightContainer.AttachPanel(&mScoresPanel);

    mPanelsContainer.AttachPanel(&mTopLeftContainer);
    mPanelsContainer.AttachPanel(&mTopMiddleContainer);
    mPanelsContainer.AttachPanel(&mTopRightContainer);
    mPanelsContainer.AttachPanel(&mBigFontMessage);

    mPanelsContainer.SetupHUD();

    mBigFontMessage.SetVisible(false);
    mCarNamePanel.SetVisible(false);
    mDistrictNamePanel.SetVisible(false);
    mWantedLevelPanel.SetVisible(false);
}

void HUD::UpdateFrame()
{
    Pedestrian* character = mHumanPlayer->mCharacter;

    // update weapon info
    if (character->mCurrentWeapon == eWeapon_Fists)
    {
        mWeaponPanel.SetVisible(false);
    }
    else
    {
        mWeaponPanel.SetWeapon(character->mWeapons[character->mCurrentWeapon]);
        mWeaponPanel.SetVisible(true);
    }

    // update wanted level
    int currentWantedLevel = mHumanPlayer->GetWantedLevel();
    mWantedLevelPanel.SetWantedLevel(currentWantedLevel);
    if (currentWantedLevel == 0)
    {
        mWantedLevelPanel.SetVisible(false);
    }
    else
    {
        mWantedLevelPanel.SetVisible(true);
    }

    // update scores
    mScoresPanel.SetScores(100, 4, 1); // todo: implement scores system

    // todo: implement bonus
    if (true)
    {
        mBonusPanel.SetBonus(true, character->mArmorHitPoints);
        mBonusPanel.SetVisible(true);
    }

    TickAutoHidePanels();
    mPanelsContainer.UpdateFrame();
}

void HUD::DrawFrame(GuiContext& guiContext)
{
    Rect viewportRect = guiContext.mCamera.mViewportRect;
    mPanelsContainer.SetPosition(Point(viewportRect.x, viewportRect.y));
    mPanelsContainer.SetSizeLimits(
        Point(viewportRect.w, viewportRect.h), 
        Point(viewportRect.w, viewportRect.h));
    mPanelsContainer.ComputeSize();
    mPanelsContainer.ComputePosition();
    mPanelsContainer.DrawFrame(guiContext);

    if (CheckCharacterObscure())
    {
        DrawArrowAboveCharacter(guiContext);
    }
}

void HUD::PushPagerMessage()
{
    // todo
}

void HUD::PushHelpMessage()
{
    // todo
}

void HUD::PushMissionMessage()
{
    // todo
}

void HUD::PushBombCostMessage()
{
    // todo
}

void HUD::ShowBigFontMessage(eHUDBigFontMessage messageType)
{
    // todo: move this elsewhere
    static const std::string messageIDs[] =
    {
        "2500", // MissionComplete
        "2501", // MissionFailed
        "2503", // KillFrenzy
        "2504", // FrenzyFailed
        "2505", // ExtraLifeBonus
        "8787", // Gouranga
        "4000", // YouGotIt
        "4001", // FrenzyPassed
        "4002", // BonusLost
        "4003", // Busted
        "4004", // Wasted
        "4005", // GoGoGo
    };
    if (messageType < CountOf(messageIDs))
    {
        const std::string& messageText = gGameTexts.GetText(messageIDs[messageType]);
        mBigFontMessage.SetText(messageText);
        ShowAutoHidePanel(&mBigFontMessage, gGameParams.mHudBigFontMessageShowDuration);
    }
    else
    {
        debug_assert(false);
    }
}

void HUD::ShowCarNameMessage(eVehicleModel carModel)
{
    debug_assert(carModel < eVehicle_COUNT);

    std::string textID = cxx::va("car%d", carModel);

    const std::string& messageText = gGameTexts.GetText(textID);
    mCarNamePanel.SetMessageText(messageText);
    ShowAutoHidePanel(&mCarNamePanel, gGameParams.mHudCarNameShowDuration);
}

void HUD::ShowDistrictNameMessage(int districtIndex)
{
    debug_assert(districtIndex >= 0);

    std::string textID = cxx::va("%03darea%03d", gGameMap.mStyleFileNumber, districtIndex);
    const std::string& messageText = gGameTexts.GetText(textID);
    mDistrictNamePanel.SetMessageText(messageText);
    ShowAutoHidePanel(&mDistrictNamePanel, gGameParams.mHudDistrictNameShowDuration);
}

void HUD::ShowAutoHidePanel(HUDPanel* panel, float showDuration)
{
    debug_assert(panel);
    panel->SetVisible(true);

    // check if already in list
    for (AutoHidePanel& currElement: mAutoHidePanels)
    {
        if (currElement.mPointer == panel)
        {
            currElement.mShowTime = gTimeManager.mUiTime;
            currElement.mShowDuration = showDuration;
            return;
        }
    }
    // add new entry
    mAutoHidePanels.emplace_back();
    AutoHidePanel& currElements = mAutoHidePanels.back();
    currElements.mPointer = panel;
    currElements.mShowTime = gTimeManager.mUiTime;
    currElements.mShowDuration = showDuration;
}

void HUD::TickAutoHidePanels()
{
    for (auto panels_iterator = mAutoHidePanels.begin(); panels_iterator != mAutoHidePanels.end(); )
    {
        AutoHidePanel& currElement = *panels_iterator;

        bool removeFromList = false;
        // process show timeout
        if (currElement.mPointer->IsVisible())
        {
            float showTimeEnd = (currElement.mShowTime + currElement.mShowDuration);
            if (gTimeManager.mUiTime > showTimeEnd)
            {
                currElement.mPointer->SetVisible(false);
                removeFromList = true;
            }
        }
        else
        {
            removeFromList = true;
        }

        if (removeFromList)
        {
            panels_iterator = mAutoHidePanels.erase(panels_iterator);
            continue;
        }

        ++panels_iterator;
    }
}

bool HUD::CheckCharacterObscure() const
{
    Pedestrian* pedestrian = mHumanPlayer->mCharacter;
    debug_assert(pedestrian);

    const glm::vec3& worldPosition = pedestrian->mTransform.mPosition;
    // convert to map position
    glm::ivec3 mapPosition = Convert::MetersToMapUnits(worldPosition);
    for (int currentBlockLayer = mapPosition.y; currentBlockLayer < MAP_LAYERS_COUNT; ++currentBlockLayer)
    {
        const MapBlockInfo* currBlock = gGameMap.GetBlockInfo(mapPosition.x, mapPosition.z, currentBlockLayer);
        if (currentBlockLayer == mapPosition.y)
        {
            if (currBlock->mSlopeType)
                continue;
        }

        if (currBlock->mFaces[eBlockFace_Lid] && !currBlock->mIsFlat)
            return true;
    }
    return false;
}

void HUD::DrawArrowAboveCharacter(GuiContext& guiContext)
{
    Pedestrian* pedestrian = mHumanPlayer->mCharacter;
    debug_assert(pedestrian);

    // setup arrow sprite
    Sprite2D arrowSprite;
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, eSpriteID_Arrow_Pointer, 0, arrowSprite);

    arrowSprite.mScale = HUD_SPRITE_SCALE;
    arrowSprite.mDrawOrder = eSpriteDrawOrder_HUD_Arrow;
    // set rotation
    arrowSprite.mRotateAngle = pedestrian->mTransformSmooth.mOrientation + cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);

    // convert character position to screen position
    GameCamera& gameCamera = mHumanPlayer->mPlayerView.mCamera;
    gameCamera.ProjectPointToScreen(pedestrian->mTransformSmooth.mPosition, arrowSprite.mPosition);

    guiContext.mSpriteBatch.DrawSprite(arrowSprite);
}
