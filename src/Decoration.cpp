#include "stdafx.h"
#include "Decoration.h"
#include "TimeManager.h"
#include "SpriteManager.h"

Decoration::Decoration(GameObjectID id, GameObjectInfo* gameObjectDesc) 
    : GameObject(eGameObjectClass_Decoration, id)
{
    if (gameObjectDesc)
    {
        mAnimationState.mAnimDesc = gameObjectDesc->mAnimationData;
    }
}

Decoration::~Decoration()
{
}

void Decoration::PreDrawFrame()
{
}

void Decoration::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mAnimationState.AdvanceAnimation(deltaTime))
    {
        gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetCurrentFrame(), 0, mDrawSprite);
    }

    if (mLifeDuration > 0 && !mAnimationState.IsAnimationActive())
    {
        MarkForDeletion();
    }
}

void Decoration::DebugDraw(DebugRenderer& debugRender)
{
}

void Decoration::Spawn(const glm::vec3& position, cxx::angle_t heading)
{
    SetTransform(position, heading);

    mAnimationState.ClearState();
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
    gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetCurrentFrame(), 0, mDrawSprite);
}

void Decoration::SetLifeDuration(int numCycles)
{
    mLifeDuration = numCycles;
    mAnimationState.SetMaxRepeatCycles(numCycles);
}

void Decoration::SetTransform(const glm::vec3& position, cxx::angle_t heading)
{
    mPosition = position;
    mRotation = heading;

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
