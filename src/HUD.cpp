#include "stdafx.h"
#include "HUD.h"
#include "SpriteBatch.h"
#include "UiContext.h"
#include "Pedestrian.h"
#include "SpriteManager.h"
#include "GameMapManager.h"

void HUD::Setup(Pedestrian* character)
{
    mCharacter = character;
}

void HUD::UpdateFrame()
{

}

void HUD::DrawFrame(UiContext& uiContext)
{
    if (mCharacter == nullptr)
        return;

    // temporary
    if (mCharacter->mCurrentWeapon != eWeaponType_Fists)
    {
        int arrowSpriteIndex = 30;
        switch (mCharacter->mCurrentWeapon)
        {
            case eWeaponType_Pistol: arrowSpriteIndex = 30; break;
            case eWeaponType_Machinegun: arrowSpriteIndex = 31; break;
            case eWeaponType_Flamethrower: arrowSpriteIndex = 33; break;
            case eWeaponType_RocketLauncher: arrowSpriteIndex = 32; break;
        }

        int spriteIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Arrow, arrowSpriteIndex);

        Sprite2D sprite;
        gSpriteManager.GetSpriteTexture(GAMEOBJECT_ID_NULL, spriteIndex, 0, sprite);

        sprite.mPosition.x = 0.0f;
        sprite.mPosition.y = 0.0f;
        sprite.mHeight = 0.0f;
        sprite.mScale = 1.0f;

        uiContext.mSpriteBatch.DrawSprite(sprite);
    }
}



