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

void Decoration::PreDrawFrame()
{
}

void Decoration::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mAnimationState.UpdateFrame(deltaTime))
    {
        gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetSpriteIndex(), 0, mDrawSprite);
    }

    glm::vec3 currPosition = GetPosition();
    glm::vec3 newPosition = currPosition + (mMoveVelocity * deltaTime);
    if (newPosition != currPosition)
    {
        SetTransform(newPosition, mSpawnHeading);
    }

    if (mLifeDuration > 0 && !mAnimationState.IsActive())
    {
        MarkForDeletion();
    }
}

void Decoration::DebugDraw(DebugRenderer& debugRender)
{
}

glm::vec3 Decoration::GetPosition() const
{
    return mSpawnPosition;
}

glm::vec2 Decoration::GetPosition2() const
{
    return {mSpawnPosition.x, mSpawnPosition.z};
}

void Decoration::Spawn(const glm::vec3& position, cxx::angle_t heading)
{
    SetTransform(position, heading);

    mAnimationState.ClearState();
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
    gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetSpriteIndex(), 0, mDrawSprite);
}

void Decoration::SetLifeDuration(int numCycles)
{
    mLifeDuration = numCycles;
    mAnimationState.SetMaxRepeatCycles(numCycles);
}

void Decoration::SetTransform(const glm::vec3& position, cxx::angle_t heading)
{
    mSpawnPosition = position;
    mSpawnHeading = heading;

    cxx::angle_t decoRotation = heading - cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);

    mDrawSprite.mPosition.x = position.x;
    mDrawSprite.mPosition.y = position.z;
    mDrawSprite.mHeight = position.y;
    mDrawSprite.mRotateAngle = decoRotation;
}

void Decoration::SetDrawOrder(eSpriteDrawOrder drawOrder)
{
    mDrawSprite.mDrawOrder = drawOrder;
}

void Decoration::SetScale(float scale)
{
    mDrawSprite.mScale = scale;
}

void Decoration::SetMoveVelocity(const glm::vec3& moveVelocity)
{
    mMoveVelocity = moveVelocity;
}
