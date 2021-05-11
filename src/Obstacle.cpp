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

void Obstacle::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mAnimationState.UpdateFrame(deltaTime))
    {
        SetSprite(mAnimationState.GetSpriteIndex(), 0);
    }
}

void Obstacle::DebugDraw(DebugRenderer& debugRender)
{
}

void Obstacle::HandleSpawn()
{
    debug_assert(mGameObjectDesc);

    mRemapClut = 0;

    mAnimationState.Clear();
    mAnimationState.mAnimDesc = mGameObjectDesc->mAnimationData;

    //mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);

    SetSprite(mAnimationState.GetSpriteIndex(), 0);
}
