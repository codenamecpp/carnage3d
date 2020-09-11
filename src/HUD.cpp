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

void HUDWeaponPanel::DrawFrame(GuiContext& guiContext)
{
    if (mWeaponIcon.IsNull())
        return;

    guiContext.mSpriteBatch.DrawSprite(mWeaponIcon);
}

void HUDWeaponPanel::UpdatePanelSize(const Point& maxSize)
{
    mWeaponIcon.mPosition.x = mPanelPosition.x * 1.0f;
    mWeaponIcon.mPosition.y = mPanelPosition.y * 1.0f;

    mPanelSize.x = mWeaponIcon.mTextureRegion.mRectangle.w;
    mPanelSize.y = mWeaponIcon.mTextureRegion.mRectangle.h;
}

void HUDWeaponPanel::SetWeaponIcon(eWeaponID weaponID)
{
    // there no icon for fists
    if (weaponID == eWeapon_Fists)
    {
        mWeaponIcon.Clear();
    }
    else
    {
        WeaponInfo& weapon = gGameMap.mStyleData.mWeapons[weaponID];
        int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, weapon.mSpriteIndex);

        gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, mWeaponIcon);
        mWeaponIcon.mHeight = 0.0f;
        mWeaponIcon.mScale = 1.0f;
        mWeaponIcon.mOriginMode = Sprite2D::eOriginMode_TopLeft;
    }
}

//////////////////////////////////////////////////////////////////////////

HUDBigFontMessage::HUDBigFontMessage()
{
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

void HUDBigFontMessage::SetMesssageFont(Font* font)
{
    debug_assert(font);
    mMessageFont = font;
}

void HUDBigFontMessage::SetMessageText(const std::string& messageText)
{
    mMessageText = messageText;
}

//////////////////////////////////////////////////////////////////////////

void HUD::Setup(Pedestrian* character)
{
    mCharacter = character;

    mFont = gFontManager.GetFont("SUB2.FON");
    mBigFont = gFontManager.GetFont("BIG2.FON");

    ClearTextMessages();

    // setup hud panels    
    mBigFontMessage.SetMesssageFont(mBigFont);
    mBigFontMessage.HidePanel();

    mPanelsList.push_back(&mWeaponPanel);
    mPanelsList.push_back(&mBigFontMessage);
}

void HUD::UpdateFrame()
{
    if (mCharacter)
    {
        mWeaponPanel.SetWeaponIcon(mCharacter->mCurrentWeapon);
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

void HUD::PushCarNameMessage(eVehicleModel vehicleModel)
{
    // todo
}

void HUD::PushAreaNameMessage()
{
    // todo
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
    Point position;
    Point maxSize;

    // setup weapon panel
    if (mWeaponPanel.IsPanelVisible())
    {
        position.x = 10;
        position.y = 10;
        mWeaponPanel.SetPanelPosition(position);
        mWeaponPanel.UpdatePanelSize(maxSize);
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
