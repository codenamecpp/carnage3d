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
    if (mAnimationState.AdvanceAnimation(deltaTime))
    {
        gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetCurrentFrame(), 0, mDrawSprite);
    }
}

void Obstacle::DebugDraw(DebugRenderer& debugRender)
{
}

void Obstacle::Spawn(const glm::vec3& position, cxx::angle_t heading)
{
    mSpawnPosition = position;
    mSpawnHeading = heading;

    debug_assert(mGameObjectDesc);

    mDrawSprite.mPosition.x = position.x;
    mDrawSprite.mPosition.y = position.z;
    mDrawSprite.mHeight = position.y;
    mDrawSprite.mRotateAngle = heading;

    mAnimationState.Clear();
    mAnimationState.mAnimDesc = mGameObjectDesc->mAnimationData;
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
}

glm::vec3 Obstacle::GetCurrentPosition() const
{
    // todo: implementation
    return {mDrawSprite.mPosition.x, mDrawSprite.mHeight, mDrawSprite.mPosition.y};
}

glm::vec2 Obstacle::GetCurrentPosition2() const
{
    // todo: implementation
    return {mDrawSprite.mPosition.x, mDrawSprite.mPosition.y};
}