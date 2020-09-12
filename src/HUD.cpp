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
    // process show timeout
    if (mIsPanelVisible && mIsShownWithTimeout)
    {
        if (gTimeManager.mUiTime > (mShowTimeStart + mShowTimeDuration))
        {
            HidePanel();
        }
    }
}

void HUDPanel::UpdatePanelSize(const Point& maxSize)
{
    // do nothing
}

bool HUDPanel::SetupHUD()
{
    // do nothing
    return true;
}

void HUDPanel::SetPanelPosition(const Point& tlcornerPosition)
{
    mPanelPosition = tlcornerPosition;
}

void HUDPanel::ShowPanel()
{
    mIsPanelVisible = true;
    mIsShownWithTimeout = false;
}

void HUDPanel::HidePanel()
{
    mIsPanelVisible = false;
    mIsShownWithTimeout = false;
}

void HUDPanel::ShowPanelWithTimeout(float timeout)
{
    mIsPanelVisible = true;
    mIsShownWithTimeout = true;
    debug_assert(timeout >= 0.0f);
    mShowTimeDuration = timeout;
    mShowTimeStart = gTimeManager.mUiTime;
}

bool HUDPanel::IsPanelVisible() const
{
    return mIsPanelVisible;
}

//////////////////////////////////////////////////////////////////////////

HUDWeaponPanel::HUDWeaponPanel()
{
}

bool HUDWeaponPanel::SetupHUD()
{
    mWeaponIcon.mHeight = 0.0f;
    mWeaponIcon.mScale = 1.0f;
    mWeaponIcon.mOriginMode = Sprite2D::eOriginMode_TopLeft;
    return true;
}

void HUDWeaponPanel::DrawFrame(GuiContext& guiContext)
{
    if (mWeaponIcon.IsNull())
        return;

    mWeaponIcon.mPosition.x = mPanelPosition.x * 1.0f;
    mWeaponIcon.mPosition.y = mPanelPosition.y * 1.0f;

    guiContext.mSpriteBatch.DrawSprite(mWeaponIcon);
}

void HUDWeaponPanel::UpdatePanelSize(const Point& maxSize)
{
    mPanelSize.x = mWeaponIcon.mTextureRegion.mRectangle.w;
    mPanelSize.y = mWeaponIcon.mTextureRegion.mRectangle.h;
}

void HUDWeaponPanel::SetWeaponIcon(eWeaponID weaponID)
{
    WeaponInfo& weapon = gGameMap.mStyleData.mWeapons[weaponID];
    int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, weapon.mSpriteIndex);

    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mWeaponIcon);
}

//////////////////////////////////////////////////////////////////////////

HUDBigFontMessage::HUDBigFontMessage()
{
}

bool HUDBigFontMessage::SetupHUD()
{
    mMessageFont = gFontManager.GetFont("BIG2.FON");
    debug_assert(mMessageFont);

    return true;
}

void HUDBigFontMessage::DrawFrame(GuiContext& guiContext)
{   
    if (mMessageFont)
    {
        int fontPaletteIndex = gGameMap.mStyleData.GetFontPaletteIndex(0);
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

bool HUDCarNamePanel::SetupHUD()
{
    mMessageFont = gFontManager.GetFont("SUB2.FON");
    debug_assert(mMessageFont);

    int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, eSpriteID_Arrow_VehicleDisplay);
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mBackgroundSprite);

    mBackgroundSprite.mHeight = 0.0f;
    mBackgroundSprite.mScale = 1.0f;
    mBackgroundSprite.mOriginMode = Sprite2D::eOriginMode_TopLeft;
    return true;
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

        int fontPaletteIndex = gGameMap.mStyleData.GetFontPaletteIndex(0);
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

bool HUDDistrictNamePanel::SetupHUD()
{
    mMessageFont = gFontManager.GetFont("SUB2.FON");
    debug_assert(mMessageFont);
    // setup left part sprite
    int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, eSpriteID_Arrow_AreaDisplayLeft);
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mBackgroundSpriteLeftPart);
    mBackgroundSpriteLeftPart.mHeight = 0.0f;
    mBackgroundSpriteLeftPart.mScale = 1.0f;
    mBackgroundSpriteLeftPart.mOriginMode = Sprite2D::eOriginMode_TopLeft;
    // setup right part sprite
    spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, eSpriteID_Arrow_AreaDisplayRight);
    gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mBackgroundSpriteRightPart);
    mBackgroundSpriteRightPart.mHeight = 0.0f;
    mBackgroundSpriteRightPart.mScale = 1.0f;
    mBackgroundSpriteRightPart.mOriginMode = Sprite2D::eOriginMode_TopLeft;
    return true;
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

        int fontPaletteIndex = gGameMap.mStyleData.GetFontPaletteIndex(0);
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

void HUD::Setup(Pedestrian* character)
{
    mCharacter = character;

    ClearTextMessages();

    // setup hud panels
    mWeaponPanel.SetupHUD();

    mBigFontMessage.SetupHUD();
    mBigFontMessage.HidePanel();

    mCarNamePanel.SetupHUD();
    mCarNamePanel.HidePanel();

    mDistrictNamePanel.SetupHUD();
    mDistrictNamePanel.HidePanel();

    mPanelsList.clear();
    mPanelsList.push_back(&mWeaponPanel);
    mPanelsList.push_back(&mBigFontMessage);
    mPanelsList.push_back(&mCarNamePanel);
    mPanelsList.push_back(&mDistrictNamePanel);
}

void HUD::UpdateFrame()
{
    if (mCharacter)
    {
        if (mCharacter->mCurrentWeapon == eWeapon_Fists)
        {
            mWeaponPanel.HidePanel();
        }
        else
        {
            mWeaponPanel.SetWeaponIcon(mCharacter->mCurrentWeapon);
            mWeaponPanel.ShowPanel();
        }
    }

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
    UpdatePanelsLayout(guiContext.mCamera.mViewportRect);

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

void HUD::ClearTextMessages()
{
    // todo
}

void HUD::UpdatePanelsLayout(const Rect& viewportRect)
{
    const int yoffset = 10; // todo: magic numbers

    Point position;
    Point maxSize;

    // setup weapon panel
    if (mWeaponPanel.IsPanelVisible())
    {
        position.x = 10;
        position.y = yoffset;
        mWeaponPanel.SetPanelPosition(position);
        mWeaponPanel.UpdatePanelSize(Point());
    }

    // setup big font message
    if (mBigFontMessage.IsPanelVisible())
    {
        maxSize.x = viewportRect.w / 2;
        maxSize.y = 0;
        mBigFontMessage.UpdatePanelSize(maxSize);
        position.x = viewportRect.w / 2 - mBigFontMessage.mPanelSize.x / 2;
        position.y = viewportRect.h / 4 - mBigFontMessage.mPanelSize.y / 2;
        mBigFontMessage.SetPanelPosition(position);
    }

    // middle top
    {
        position.y = yoffset; // base offset
        if (mDistrictNamePanel.IsPanelVisible())
        {
            mDistrictNamePanel.UpdatePanelSize(Point());
            position.x = viewportRect.w / 2 - mDistrictNamePanel.mPanelSize.x / 2;
            mDistrictNamePanel.SetPanelPosition(position);
            position.y += mDistrictNamePanel.mPanelSize.y;
            position.y += yoffset;
        }

        if (mCarNamePanel.IsPanelVisible())
        {
            mCarNamePanel.UpdatePanelSize(Point());
            position.x = viewportRect.w / 2 - mCarNamePanel.mPanelSize.x / 2;
            mCarNamePanel.SetPanelPosition(position);
            position.y += mCarNamePanel.mPanelSize.y;
            position.y += yoffset;
        }
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
        mBigFontMessage.ShowPanelWithTimeout(gGameParams.mHudBigFontMessageShowDuration);
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
    mCarNamePanel.ShowPanelWithTimeout(gGameParams.mHudCarNameShowDuration);
}

void HUD::ShowDistrictNameMessage(int districtIndex)
{
    debug_assert(districtIndex >= 0);

    std::string textID = cxx::va("%03darea%03d", gGameMap.mStyleData.GetStyleNumber(), districtIndex);
    const std::string& messageText = gGameTexts.GetText(textID);
    mDistrictNamePanel.SetMessageText(messageText);
    mDistrictNamePanel.ShowPanelWithTimeout(gGameParams.mHudDistrictNameShowDuration);
}
