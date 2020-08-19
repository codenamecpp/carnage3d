#include "stdafx.h"
#include "Decoration.h"
#include "TimeManager.h"
#include "SpriteManager.h"
#include "GameCheatsWindow.h"

Decoration::Decoration(GameObjectID id, GameObjectStyle* desc) 
    : GameObject(eGameObjectClass_Decoration, id)
    , mGameObjectDesc(desc)
{
    debug_assert(mGameObjectDesc);
}

Decoration::~Decoration()
{
}

void Decoration::DrawFrame(SpriteBatch& spriteBatch)
{
    if (!gGameCheatsWindow.mEnableDrawDecorations)
        return;

    gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetCurrentFrame(), 0, mDrawSprite);
    mDrawSprite.SetOriginToCenter();
    spriteBatch.DrawSprite(mDrawSprite);
}

void Decoration::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    mAnimationState.AdvanceAnimation(deltaTime);
}

void Decoration::DrawDebug(DebugRenderer& debugRender)
{
}

void Decoration::Spawn(const glm::vec3& startPosition, cxx::angle_t startRotation)
{
    debug_assert(mGameObjectDesc);

    mDrawSprite.mPosition.x = startPosition.x;
    mDrawSprite.mPosition.y = startPosition.z;
    mDrawSprite.mHeight = startPosition.y + 0.1f;
    mDrawSprite.mRotateAngle = startRotation;

    mAnimationState.SetNull();
    mAnimationState.mAnimDesc = mGameObjectDesc->mAnimationData;
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
}
