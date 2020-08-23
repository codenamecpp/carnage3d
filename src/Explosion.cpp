#include "stdafx.h"
#include "Explosion.h"
#include "TimeManager.h"
#include "SpriteManager.h"
#include "DebugRenderer.h"
#include "PhysicsManager.h"
#include "Pedestrian.h"
#include "Vehicle.h"

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

    if (!IsDamageDone())
    {
        ProcessDamage();
    }

    if (!mAnimationState.IsAnimationActive())
    {
        MarkForDeletion();
    }
}

void Explosion::DrawDebug(DebugRenderer& debugRender)
{
    float damageRadiusA = gGameParams.mExplosionPrimaryDamageDistance;
    float damageRadiusB = gGameParams.mExplosionSecondaryDamageDistance;

    debugRender.DrawSphere(mExplosionEpicentre, damageRadiusA, Color32_Red, false);
    debugRender.DrawSphere(mExplosionEpicentre, damageRadiusB, Color32_Yellow, false);
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

    mExplosionEpicentre = startPosition;
}

bool Explosion::IsDamageDone() const
{
    return mDamageDone;
}

void Explosion::ProcessDamage()
{
    float maxImpactRadius = gGameParams.mExplosionPrimaryDamageDistance;
    float minImpactRadius = gGameParams.mExplosionSecondaryDamageDistance;

    float impactRadius = std::max(maxImpactRadius, minImpactRadius);

    glm::vec2 centerPoint (mExplosionEpicentre.x, mExplosionEpicentre.z);
    glm::vec2 extents (
        impactRadius,
        impactRadius
    );

    // single shot
    mDamageDone = true;

    // todo: fix distance to target
    PhysicsQueryResult queryResult;
    gPhysics.QueryObjectsWithinBox(centerPoint, extents, queryResult);

    if (queryResult.IsNull())
        return;

    for (int icurr = 0; icurr < queryResult.mElementsCount; ++icurr)
    {
        PhysicsQueryElement& currElement = queryResult.mElements[icurr];

        if (CarPhysicsBody* carPhysics = currElement.mCarComponent)
        {
            // todo: temporary implementation
            carPhysics->mReferenceCar->ReceiveDamage(gGameParams.mExplosionPrimaryDamage);
            continue;
        }

        if (PedPhysicsBody* pedPhysics = currElement.mPedComponent)
        {
            // todo: temporary implementation
            pedPhysics->mReferencePed->ReceiveDamageFromExplosion(this);
            continue;
        }
    }
}
