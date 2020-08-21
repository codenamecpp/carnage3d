#include "stdafx.h"
#include "Explosion.h"
#include "TimeManager.h"
#include "SpriteManager.h"

Explosion::Explosion(GameObjectID id) 
    : GameObject(eGameObjectClass_Explosion, id)
{
}

Explosion::~Explosion()
{
}

void Explosion::PreDrawFrame()
{
}

void Explosion::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mAnimationState.AdvanceAnimation(deltaTime))
    {
        gSpriteManager.GetExplosionTexture(mAnimationState.GetCurrentFrame(), mDrawSprite);
        mDrawSprite.SetOriginToCenter();
    }

    if (!mAnimationState.IsAnimationActive())
    {
        MarkForDeletion();
    }
}

void Explosion::DrawDebug(DebugRenderer& debugRender)
{
}

void Explosion::Spawn(const glm::vec3& startPosition)
{
    mDrawSprite.mPosition.x = startPosition.x;
    mDrawSprite.mPosition.y = startPosition.z;
    mDrawSprite.mHeight = startPosition.y;

    mAnimationState.Clear();
    // todo
    int numFrames = gSpriteManager.GetExplosionFramesCount();
    mAnimationState.mAnimDesc.Setup(0, numFrames);
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
    mAnimationState.SetMaxRepeatCycles(1);

    if (!gSpriteManager.GetExplosionTexture(0, mDrawSprite))
    {
        debug_assert(false);
    }
    mDrawSprite.SetOriginToCenter();
}