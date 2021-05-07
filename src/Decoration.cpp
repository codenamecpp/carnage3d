#include "stdafx.h"
#include "Decoration.h"
#include "TimeManager.h"
#include "SpriteManager.h"

Decoration::Decoration(GameObjectID id, GameObjectInfo* gameObjectDesc) 
    : GameObject(eGameObjectClass_Decoration, id)
    , mMoveVelocity()
    , mAnimationState()
{
    if (gameObjectDesc)
    {
        mAnimationState.mAnimDesc = gameObjectDesc->mAnimationData;
        mDrawSprite.mDrawOrder = gameObjectDesc->mDrawOrder;
    }
}

void Decoration::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mAnimationState.UpdateFrame(deltaTime))
    {
        gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetSpriteIndex(), 0, mDrawSprite);
        RefreshDrawSprite();
    }

    glm::vec3 newPosition = mTransform.mPosition + (mMoveVelocity * deltaTime);
    if (newPosition != mTransform.mPosition)
    {
        SetTransform(newPosition, mTransform.mOrientation);
    }

    if (mLifeDuration > 0 && !mAnimationState.IsActive())
    {
        MarkForDeletion();
    }
}

void Decoration::DebugDraw(DebugRenderer& debugRender)
{
}

void Decoration::HandleSpawn()
{
    mAnimationState.ClearState();
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
    gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetSpriteIndex(), 0, mDrawSprite);
    RefreshDrawSprite();
}

void Decoration::SetLifeDuration(int numCycles)
{
    mLifeDuration = numCycles;
    mAnimationState.SetMaxRepeatCycles(numCycles);
}

void Decoration::SetDrawOrder(eSpriteDrawOrder drawOrder)
{
    mDrawSprite.mDrawOrder = drawOrder;
}

void Decoration::SetScale(float scale)
{
    mDrawSprite.mScale = scale;
    RefreshDrawSprite();
}

void Decoration::SetMoveVelocity(const glm::vec3& moveVelocity)
{
    mMoveVelocity = moveVelocity;
}