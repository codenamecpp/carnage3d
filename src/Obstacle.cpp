#include "stdafx.h"
#include "Obstacle.h"
#include "TimeManager.h"
#include "SpriteManager.h"
#include "GameCheatsWindow.h"

Obstacle::Obstacle(GameObjectID id, GameObjectStyle* desc) 
    : GameObject(eGameObjectClass_Obstacle, id)
    , mGameObjectDesc(desc)
{
    debug_assert(mGameObjectDesc);
}

Obstacle::~Obstacle()
{
}

void Obstacle::DrawFrame(SpriteBatch& spriteBatch)
{
    if (!gGameCheatsWindow.mEnableDrawObstacles)
        return;

    gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetCurrentFrame(), 0, mDrawSprite);
    mDrawSprite.SetOriginToCenter();
    spriteBatch.DrawSprite(mDrawSprite);
}

void Obstacle::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    mAnimationState.AdvanceAnimation(deltaTime);
}

void Obstacle::DrawDebug(DebugRenderer& debugRender)
{
}

void Obstacle::Spawn(const glm::vec3& startPosition, cxx::angle_t startRotation)
{
    debug_assert(mGameObjectDesc);

    mDrawSprite.mPosition.x = startPosition.x;
    mDrawSprite.mPosition.y = startPosition.z;
    mDrawSprite.mHeight = startPosition.y;
    mDrawSprite.mRotateAngle = startRotation;

    mAnimationState.SetNull();
    mAnimationState.mAnimDesc = mGameObjectDesc->mAnimationData;
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
}
