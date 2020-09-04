#include "stdafx.h"
#include "HUD.h"
#include "SpriteBatch.h"
#include "GuiContext.h"
#include "Pedestrian.h"
#include "SpriteManager.h"
#include "GameMapManager.h"
#include "FontManager.h"
#include "Font.h"

void HUD::Setup(Pedestrian* character)
{
    mCharacter = character;
    mFont = gFontManager.GetFont("SUB2.FON");

    ClearTextMessages();
}

void HUD::UpdateFrame()
{

}

void HUD::DrawFrame(GuiContext& guiContext)
{
    if (mCharacter == nullptr)
        return;

    // temporary
    if (mCharacter->mCurrentWeapon != eWeapon_Fists)
    {
        WeaponInfo& weapon = gGameMap.mStyleData.mWeapons[mCharacter->mCurrentWeapon];
        int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, weapon.mSpriteIndex);

        Sprite2D sprite;
        gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, sprite);

        sprite.mPosition.x = 10.0f;
        sprite.mPosition.y = 10.0f;
        sprite.mHeight = 0.0f;
        sprite.mScale = 1.0f;
        sprite.mOriginMode = Sprite2D::eOriginMode_TopLeft;

        guiContext.mSpriteBatch.DrawSprite(sprite);
    }

    if (mFont)
    {
        int fontPaletteIndex = gGameMap.mStyleData.GetFontPaletteIndex(0);

        ePedestrianState currStateId = mCharacter->GetCurrentStateID();
        std::string currFpsString = cxx::va("Current state: %s", cxx::enum_to_string(currStateId));

        Point textDims;
        mFont->MeasureString(currFpsString, textDims);

        Point textPos;
        textPos.x = guiContext.GetScreenSizex() - textDims.x - 10;
        textPos.y = 10;

        mFont->DrawString(guiContext, currFpsString, textPos, fontPaletteIndex);
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