#include "stdafx.h"
#include "Obstacle.h"
#include "TimeManager.h"
#include "SpriteManager.h"

Obstacle::Obstacle(GameObjectID id, GameObjectInfo* desc) 
    : GameObject(eGameObjectClass_Obstacle, id)
    , mGameObjectDesc(desc)
{
    debug_assert(mGameObjectDesc);
    if (mGameObjectDesc)
    {
        mDrawSprite.mDrawOrder = mGameObjectDesc->mDrawOrder;
    }
}

Obstacle::~Obstacle()
{
}

void Obstacle::PreDrawFrame()
{
}

void Obstacle::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mAnimationState.UpdateFrame(deltaTime))
    {
        gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetSpriteIndex(), 0, mDrawSprite);
    }
}

void Obstacle::DebugDraw(DebugRenderer& debugRender)
{
}

void Obstacle::OnGameObjectSpawn()
{
    debug_assert(mGameObjectDesc);

    mDrawSprite.mPosition.x = mSpawnPosition.x;
    mDrawSprite.mPosition.y = mSpawnPosition.z;
    mDrawSprite.mHeight = mSpawnPosition.y;
    mDrawSprite.mRotateAngle = mSpawnHeading;

    mAnimationState.Clear();
    mAnimationState.mAnimDesc = mGameObjectDesc->mAnimationData;
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
}

glm::vec3 Obstacle::GetPosition() const
{
    // todo: implementation
    return {mDrawSprite.mPosition.x, mDrawSprite.mHeight, mDrawSprite.mPosition.y};
}

glm::vec2 Obstacle::GetPosition2() const
{
    // todo: implementation
    return {mDrawSprite.mPosition.x, mDrawSprite.mPosition.y};
}