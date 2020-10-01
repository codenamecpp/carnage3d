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
    : mPanelPosition()
    , mPanelSize()
{
}

HUDPanel::~HUDPanel()
{
}

void HUDPanel::DrawFrame(GuiContext& guiContext)
{
    // do nothing
}

void HUDPanel::UpdateFrame()
{
    // do nothing
}

void HUDPanel::UpdatePanelSize(const Point& maxSize)
{
    // do nothing
}

void HUDPanel::SetupHUD()
{
    // do nothing
}

void HUDPanel::ShowPanel()
{
    mIsPanelVisible = true;
}

void HUDPanel::HidePanel()
{
    mIsPanelVisible = false;
}

bool HUDPanel::IsPanelVisible() const
{
    return mIsPanelVisible;
}

//////////////////////////////////////////////////////////////////////////

HUDWeaponPanel::HUDWeaponPanel()
{
}

void HUDWeaponPanel::SetupHUD()
{
    mWeaponIcon.mHeight = 0.0f;
    mWeaponIcon.mScale = HUD_SPRITE_SCALE;
    mWeaponIcon.mOriginMode = Sprite2D::eOriginMode_TopLeft;

    mAmmunitionFont = gFontManager.GetFont("SUB1.FON");
    debug_assert(mAmmunitionFont);
}

void HUDWeaponPanel::DrawFrame(GuiContext& guiContext)
{
    if (mWeaponIcon.IsNull())
        return;

    mWeaponIcon.mPosition.x = mPanelPosition.x * 1.0f;
    mWeaponIcon.mPosition.y = mPanelPosition.y * 1.0f;

    guiContext.mSpriteBatch.DrawSprite(mWeaponIcon);

    if (mAmmunitionFont)
    {
        Point textDims;
        mAmmunitionFont->MeasureString(mAmmunitionText, textDims);

        Point textPos 
        {
            (mPanelPosition.x + mPanelSize.x) - textDims.x,
            (mPanelPosition.y + mPanelSize.y) - textDims.y + 4,
        };
        int fontPaletteIndex = gGameMap.mStyleData.GetFontPaletteIndex(FontRemap_Default);
        mAmmunitionFont->DrawString(guiContext, mAmmunitionText, textPos, mPanelSize, fontPaletteIndex);
    }
}

void HUDWeaponPanel::UpdatePanelSize(const Point& maxSize)
{
    mPanelSize.x = mWeaponIcon.mTextureRegion.mRectangle.w;
    mPanelSize.y = mWeaponIcon.mTextureRegion.mRectangle.h;
}

void HUDWeaponPanel::SetWeaponInfo(Weapon& weaponState)
{
    if ((mPrevAmmunitionCount == 0) || (mPrevAmmunitionCount != weaponState.mAmmunition))
    {
        mPrevAmmunitionCount = weaponState.mAmmunition;
        mAmmunitionText = cxx::va("%d", weaponState.mAmmunition);
    }

    WeaponInfo* weaponInfo = weaponState.GetWeaponInfo();
    int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, weaponInfo->mSpriteIndex);

    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mWeaponIcon);
}

//////////////////////////////////////////////////////////////////////////

HUDBigFontMessage::HUDBigFontMessage()
{
}

void HUDBigFontMessage::SetupHUD()
{
    mMessageFont = gFontManager.GetFont("BIG2.FON");
    debug_assert(mMessageFont);
}

void HUDBigFontMessage::DrawFrame(GuiContext& guiContext)
{   
    if (mMessageFont)
    {
        int fontPaletteIndex = gGameMap.mStyleData.GetFontPaletteIndex(FontRemap_Default);
        mMessageFont->DrawString(guiContext, mMessageText, mPanelPosition, mPanelSize, fontPaletteIndex);
    }
}

void HUDBigFontMessage::UpdatePanelSize(const Point& maxSize)
{
    if (mMessageFont)
    {
        mMessageFont->MeasureString(mMessageText, maxSize, mPanelSize);
    }
}

void HUDBigFontMessage::SetMessageText(const std::string& messageText)
{
    mMessageText = messageText;
}

//////////////////////////////////////////////////////////////////////////

HUDCarNamePanel::HUDCarNamePanel()
{
}

void HUDCarNamePanel::SetMessageText(const std::string& messageText)
{
    mMessageText = messageText;
}

void HUDCarNamePanel::SetupHUD()
{
    mMessageFont = gFontManager.GetFont("SUB2.FON");
    debug_assert(mMessageFont);

    int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, eSpriteID_Arrow_VehicleDisplay);
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mBackgroundSprite);

    mBackgroundSprite.mHeight = 0.0f;
    mBackgroundSprite.mScale = HUD_SPRITE_SCALE;
    mBackgroundSprite.mOriginMode = Sprite2D::eOriginMode_TopLeft;
}

void HUDCarNamePanel::DrawFrame(GuiContext& guiContext)
{
    mBackgroundSprite.mPosition.x = mPanelPosition.x * 1.0f;
    mBackgroundSprite.mPosition.y = mPanelPosition.y * 1.0f;

    guiContext.mSpriteBatch.DrawSprite(mBackgroundSprite);
    if (mMessageFont)
    {
        Point textDims;
        mMessageFont->MeasureString(mMessageText, textDims);

        Point textPosition = mPanelPosition;
        textPosition.x = mPanelPosition.x + (mPanelSize.x / 2) - (textDims.x / 2);
        textPosition.y = mPanelPosition.y + (mPanelSize.y / 2) - (textDims.y / 2);

        int fontPaletteIndex = gGameMap.mStyleData.GetFontPaletteIndex(FontRemap_Default);
        mMessageFont->DrawString(guiContext, mMessageText, textPosition, fontPaletteIndex);
    }
}

void HUDCarNamePanel::UpdatePanelSize(const Point& maxSize)
{
    mPanelSize.x = mBackgroundSprite.mTextureRegion.mRectangle.w;
    mPanelSize.y = mBackgroundSprite.mTextureRegion.mRectangle.h;
}

//////////////////////////////////////////////////////////////////////////

HUDDistrictNamePanel::HUDDistrictNamePanel()
{
}

void HUDDistrictNamePanel::SetMessageText(const std::string& messageText)
{
    mMessageText = messageText;
}

void HUDDistrictNamePanel::SetupHUD()
{
    mMessageFont = gFontManager.GetFont("SUB2.FON");
    debug_assert(mMessageFont);
    // setup left part sprite
    int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, eSpriteID_Arrow_AreaDisplayLeft);
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mBackgroundSpriteLeftPart);
    mBackgroundSpriteLeftPart.mHeight = 0.0f;
    mBackgroundSpriteLeftPart.mScale = HUD_SPRITE_SCALE;
    mBackgroundSpriteLeftPart.mOriginMode = Sprite2D::eOriginMode_TopLeft;
    // setup right part sprite
    spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, eSpriteID_Arrow_AreaDisplayRight);
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mBackgroundSpriteRightPart);
    mBackgroundSpriteRightPart.mHeight = 0.0f;
    mBackgroundSpriteRightPart.mScale = HUD_SPRITE_SCALE;
    mBackgroundSpriteRightPart.mOriginMode = Sprite2D::eOriginMode_TopLeft;
}

void HUDDistrictNamePanel::DrawFrame(GuiContext& guiContext)
{
    mBackgroundSpriteLeftPart.mPosition.x = (mPanelPosition.x * 1.0f);
    mBackgroundSpriteLeftPart.mPosition.y = (mPanelPosition.y * 1.0f);
    guiContext.mSpriteBatch.DrawSprite(mBackgroundSpriteLeftPart);

    mBackgroundSpriteRightPart.mPosition.x = (mPanelPosition.x * 1.0f) + mBackgroundSpriteLeftPart.mTextureRegion.mRectangle.w;
    mBackgroundSpriteRightPart.mPosition.y = (mPanelPosition.y * 1.0f);
    guiContext.mSpriteBatch.DrawSprite(mBackgroundSpriteRightPart);

    if (mMessageFont)
    {
        Point textDims;
        mMessageFont->MeasureString(mMessageText, textDims);

        Point textPosition = mPanelPosition;
        textPosition.x = mPanelPosition.x + (mPanelSize.x / 2) - (textDims.x / 2);
        textPosition.y = mPanelPosition.y + (mPanelSize.y / 2) - (textDims.y / 2);

        int fontPaletteIndex = gGameMap.mStyleData.GetFontPaletteIndex(FontRemap_Default);
        mMessageFont->DrawString(guiContext, mMessageText, textPosition, fontPaletteIndex);
    }
}

void HUDDistrictNamePanel::UpdatePanelSize(const Point& maxSize)
{
    mPanelSize.x = mBackgroundSpriteLeftPart.mTextureRegion.mRectangle.w + mBackgroundSpriteRightPart.mTextureRegion.mRectangle.w;
    mPanelSize.y = mBackgroundSpriteLeftPart.mTextureRegion.mRectangle.h;
    debug_assert(mBackgroundSpriteLeftPart.mTextureRegion.mRectangle.h + mBackgroundSpriteRightPart.mTextureRegion.mRectangle.h);
}

//////////////////////////////////////////////////////////////////////////

HUDWantedLevelPanel::HUDWantedLevelPanel()
{
}

void HUDWantedLevelPanel::SetWantedLevel(int wantedLevel)
{
    debug_assert(mCopSpritesCount <= GAME_MAX_WANTED_LEVEL);
    mCopSpritesCount = wantedLevel;
}

void HUDWantedLevelPanel::SetupHUD()
{
    for (CopSprite& currCopSprite: mCopSprites)
    {
        currCopSprite.mHeight = 0.0f;
        currCopSprite.mScale = HUD_SPRITE_SCALE;
        currCopSprite.mOriginMode = Sprite2D::eOriginMode_TopLeft;

        currCopSprite.mAnimationState.Clear();
        currCopSprite.mAnimationState.mAnimDesc.SetFrames(
            {
                eSpriteID_Arrow_WantedFrame1,
                eSpriteID_Arrow_WantedFrame2
            });
        currCopSprite.mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);

        // initial sprite
        gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, eSpriteID_Arrow_WantedFrame1, 0, currCopSprite);
    }
}

void HUDWantedLevelPanel::DrawFrame(GuiContext& guiContext)
{
    for (int icurr = 0; icurr < mCopSpritesCount; ++icurr)
    {
        Sprite2D& currSprite = mCopSprites[icurr];
        currSprite.mPosition.x = (mPanelPosition.x * 1.0f) + icurr * currSprite.mTextureRegion.mRectangle.w;
        currSprite.mPosition.y = (mPanelPosition.y * 1.0f);
        guiContext.mSpriteBatch.DrawSprite(currSprite);
    }
}

void HUDWantedLevelPanel::UpdateFrame()
{
    // process animation
    for (int icurr = 0; icurr < mCopSpritesCount; ++icurr)
    {
        CopSprite& currCopSprite = mCopSprites[icurr];
        if (currCopSprite.mAnimationState.UpdateFrame(gTimeManager.mUiFrameDelta))
        {
            gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, currCopSprite.mAnimationState.GetSpriteIndex(), 0, currCopSprite);
        }
    }
}

void HUDWantedLevelPanel::UpdatePanelSize(const Point& maxSize)
{
    CopSprite& currCopSprite = mCopSprites[0];
    mPanelSize.x = mCopSpritesCount * currCopSprite.mTextureRegion.mRectangle.w;
    mPanelSize.y = currCopSprite.mTextureRegion.mRectangle.h;
}

//////////////////////////////////////////////////////////////////////////

HUDScoresPanel::HUDScoresPanel()
{
}

void HUDScoresPanel::SetScores(int score, int lives, int multiplier)
{
    if (score != mPrevScore)
    {
        mPrevScore = score;
        mScoreText = cxx::va("%d", score);
    }

    if (lives != mPrevLives)
    {
        mPrevLives = lives;
        mLivesText = cxx::va(":%d", lives);
    }

    if (multiplier != mPrevMultiplier)
    {
        mPrevMultiplier = multiplier;
        mMultiplierText = cxx::va(":%d", multiplier);
    }
}

void HUDScoresPanel::SetupHUD()
{
    mFontScore = gFontManager.GetFont("SCORE2.FON");
    debug_assert(mFontScore);
    if (mFontScore)
    {
        mFontScore->SetFontBaseCharCode('0');
    }

    mFontLives = gFontManager.GetFont("MISSMUL2.FON");
    debug_assert(mFontLives);
    if (mFontLives)
    {
        mFontLives->SetFontBaseCharCode('0');
    }

    mFontMultiplier = gFontManager.GetFont("MISSMUL2.FON");
    debug_assert(mFontMultiplier);
    if (mFontMultiplier)
    {
        mFontMultiplier->SetFontBaseCharCode('0');
    }
}

void HUDScoresPanel::DrawFrame(GuiContext& guiContext)
{
    int standardPalette = gGameMap.mStyleData.GetFontPaletteIndex(FontRemap_Default);
    int livesPalette = gGameMap.mStyleData.GetFontPaletteIndex(FontRemap_Green);
    if (mFontScore)
    {
        Point pos {mPanelPosition.x + mColumnCountersDims.x, mPanelPosition.y};
        mFontScore->DrawString(guiContext, mScoreText, pos, standardPalette);
    }
    int yoffset = 0;
    if (mFontLives)
    {
        Point pos {mPanelPosition.x, mPanelPosition.y};
        mFontLives->DrawString(guiContext, mLivesText, pos, livesPalette);
        yoffset = mFontLives->GetLineHeight();
    }
    if (mFontMultiplier)
    {
        Point pos {mPanelPosition.x, mPanelPosition.y + yoffset};
        mFontMultiplier->DrawString(guiContext, mMultiplierText, pos, standardPalette);
    }
}

void HUDScoresPanel::UpdateFrame()
{
    // todo: implement scrolling effect for score digits
}

void HUDScoresPanel::UpdatePanelSize(const Point& maxSize)
{
    Point scoreDims {0, 0};
    Point livesDims {0, 0};
    Point multiplierDims {0, 0};

    mColumnCountersDims.y = 0;
    if (mFontScore)
    {
        mFontScore->MeasureString(mScoreText, scoreDims);
    }
    if (mFontLives)
    {
        mFontLives->MeasureString(mLivesText, livesDims);
        mColumnCountersDims.y += mFontLives->GetLineHeight();
    }
    if (mFontMultiplier)
    {
        mFontMultiplier->MeasureString(mMultiplierText, multiplierDims);
        mColumnCountersDims.y += mFontMultiplier->GetLineHeight();
    }
    mColumnCountersDims.x = std::max(livesDims.x, multiplierDims.x);
    mPanelSize.x = scoreDims.x + mColumnCountersDims.x;
    mPanelSize.y = std::max(scoreDims.y, mColumnCountersDims.y);
}

//////////////////////////////////////////////////////////////////////////

void HUD::SetupHUD(HumanPlayer* humanPlayer)
{
    mHumanPlayer = humanPlayer;
    debug_assert(mHumanPlayer);

    mAutoHidePanels.clear();
    mTextMessagesQueue.clear();

    // setup hud panels
    mWeaponPanel.SetupHUD();

    mBigFontMessage.SetupHUD();
    mBigFontMessage.HidePanel();

    mCarNamePanel.SetupHUD();
    mCarNamePanel.HidePanel();

    mDistrictNamePanel.SetupHUD();
    mDistrictNamePanel.HidePanel();

    mWantedLevelPanel.SetupHUD();
    mWantedLevelPanel.HidePanel();

    mScoresPanel.SetupHUD();

    mPanelsList.clear();
    mPanelsList.push_back(&mWeaponPanel);
    mPanelsList.push_back(&mBigFontMessage);
    mPanelsList.push_back(&mCarNamePanel);
    mPanelsList.push_back(&mDistrictNamePanel);
    mPanelsList.push_back(&mWantedLevelPanel);
    mPanelsList.push_back(&mScoresPanel);
}

void HUD::UpdateFrame()
{
    Pedestrian* character = mHumanPlayer->mCharacter;

    // update weapon info
    if (character->mCurrentWeapon == eWeapon_Fists)
    {
        mWeaponPanel.HidePanel();
    }
    else
    {
        mWeaponPanel.SetWeaponInfo(character->mWeapons[character->mCurrentWeapon]);
        mWeaponPanel.ShowPanel();
    }

    // update wanted level
    int currentWantedLevel = mHumanPlayer->GetWantedLevel();
    mWantedLevelPanel.SetWantedLevel(currentWantedLevel);
    if (currentWantedLevel == 0)
    {
        mWantedLevelPanel.HidePanel();
    }
    else
    {
        mWantedLevelPanel.ShowPanel();
    }

    // update scores
    mScoresPanel.SetScores(100, 4, 1); // todo: implement scores system

    TickAutoHidePanels();

    // update hud panels
    for (HUDPanel* currentPanel: mPanelsList)
    {
        if (currentPanel->IsPanelVisible())
        {
            currentPanel->UpdateFrame();
        }
    }
}

void HUD::DrawFrame(GuiContext& guiContext)
{
    // pre draw
    ArrangePanels(guiContext.mCamera.mViewportRect);

    // draw hud panels
    for (HUDPanel* currentPanel: mPanelsList)
    {
        if (currentPanel->IsPanelVisible())
        {
            currentPanel->DrawFrame(guiContext);
        }
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

void HUD::ArrangePanels(const Rect& viewportRect)
{
    const int screen_offset = 10; // todo: magic numbers

    // top left
    {
        Rect bounds (screen_offset, screen_offset, viewportRect.w - screen_offset, viewportRect.y - screen_offset);
        ArrangePanelsVert(bounds, eHUDPanelAlign_Left, screen_offset, 
            {
                &mWeaponPanel,
            });
    }

    // top center
    {
        Rect bounds (0, screen_offset, viewportRect.w, viewportRect.y - screen_offset);
        ArrangePanelsVert(bounds, eHUDPanelAlign_Center, screen_offset, 
            {
                &mWantedLevelPanel,
                &mDistrictNamePanel,
                &mCarNamePanel,
            });
    }

    // top right
    {
        Rect bounds (0, screen_offset, viewportRect.w - screen_offset, viewportRect.y - screen_offset);
        ArrangePanelsVert(bounds, eHUDPanelAlign_Right, screen_offset, 
            {
                &mScoresPanel,
            });
    }

    // setup big font message
    if (mBigFontMessage.IsPanelVisible())
    {
        Point maxSize;
        maxSize.x = viewportRect.w / 2;
        maxSize.y = 0;
        mBigFontMessage.UpdatePanelSize(maxSize);

        Point position;
        position.x = viewportRect.w / 2 - mBigFontMessage.mPanelSize.x / 2;
        position.y = viewportRect.h / 4 - mBigFontMessage.mPanelSize.y / 2;
        mBigFontMessage.SetPanelPosition(position);
    }
}

void HUD::ArrangePanelsHorz(const Rect& bounds, eHUDPanelAlign panelsAlign, int spacing, const std::initializer_list<HUDPanel*>& panels)
{
    debug_assert(false); // todo: implement
}

void HUD::ArrangePanelsVert(const Rect& bounds, eHUDPanelAlign panelsAlign, int spacing, const std::initializer_list<HUDPanel*>& panels)
{
    Point position (bounds.x, bounds.y);
    Point dimensions (bounds.w, bounds.h);

    for (HUDPanel* currPanel: panels)
    {
        if (!currPanel->IsPanelVisible())
            continue;

        currPanel->UpdatePanelSize(dimensions);
        switch (panelsAlign)
        {
            case eHUDPanelAlign_Left:
                position.x = bounds.x;
            break;
            case eHUDPanelAlign_Center:
                position.x = bounds.x + (bounds.w / 2) - (currPanel->mPanelSize.x / 2);
            break;
            case eHUDPanelAlign_Right:
                position.x = bounds.x + bounds.w - currPanel->mPanelSize.x;
            break;
            default:
                debug_assert(false);
            break;
        }
        currPanel->SetPanelPosition(position);
        position.y += currPanel->mPanelSize.y;
        position.y += spacing;
    }
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
        mBigFontMessage.SetMessageText(messageText);
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
    panel->ShowPanel();

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
        if (currElement.mPointer->IsPanelVisible())
        {
            float showTimeEnd = (currElement.mShowTime + currElement.mShowDuration);
            if (gTimeManager.mUiTime > showTimeEnd)
            {
                currElement.mPointer->HidePanel();
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