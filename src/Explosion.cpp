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
    }

    if (!IsDamageDone())
    {
        ProcessPrimaryDamage();
        ProcessSecondaryDamage();
    }

    if (!mAnimationState.IsAnimationActive())
    {
        MarkForDeletion();
    }
}

void Explosion::DebugDraw(DebugRenderer& debugRender)
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
    mDrawSprite.mDrawOrder = eSpriteDrawOrder_Explosion;

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

    mExplosionEpicentre = startPosition;
    mPrimaryDamageDone = false;
    mSecondaryDamageDone = false;
}

bool Explosion::IsDamageDone() const
{
    return mPrimaryDamageDone && mSecondaryDamageDone;
}

void Explosion::ProcessPrimaryDamage()
{
    if (mPrimaryDamageDone)
        return;

    mPrimaryDamageDone = true;

    // primary damage
    glm::vec2 centerPoint (mExplosionEpicentre.x, mExplosionEpicentre.z);
    glm::vec2 extents (
        gGameParams.mExplosionPrimaryDamageDistance,
        gGameParams.mExplosionPrimaryDamageDistance
    );

    PhysicsQueryResult queryResult;
    gPhysics.QueryObjectsWithinBox(centerPoint, extents, queryResult);

    for (int icurr = 0; icurr < queryResult.mElementsCount; ++icurr)
    {
        PhysicsQueryElement& currElement = queryResult.mElements[icurr];

        DamageInfo damageInfo;
        damageInfo.SetDamageFromExplosion(gGameParams.mExplosionPrimaryDamage, this);

        if (CarPhysicsBody* carPhysics = currElement.mCarComponent)
        {
            // todo: temporary implementation
            carPhysics->mReferenceCar->ReceiveDamage(damageInfo);
            continue;
        }

        if (PedPhysicsBody* pedPhysics = currElement.mPedComponent)
        {
            // todo: temporary implementation
            pedPhysics->mReferencePed->ReceiveDamage(damageInfo);
            continue;
        }
    }

    queryResult.Clear();
}

void Explosion::ProcessSecondaryDamage()
{
    if (mSecondaryDamageDone)
        return;

    // do secondary damage
    glm::vec2 centerPoint (mExplosionEpicentre.x, mExplosionEpicentre.z);
    glm::vec2 extents (
        gGameParams.mExplosionSecondaryDamageDistance,
        gGameParams.mExplosionSecondaryDamageDistance
    );

    PhysicsQueryResult queryResult;
    gPhysics.QueryObjectsWithinBox(centerPoint, extents, queryResult);

    for (int icurr = 0; icurr < queryResult.mElementsCount; ++icurr)
    {
        PhysicsQueryElement& currElement = queryResult.mElements[icurr];

        DamageInfo damageInfo;
        damageInfo.SetDamageFromFire(gGameParams.mExplosionSecondaryDamage, this);

        if (CarPhysicsBody* carPhysics = currElement.mCarComponent)
        {
            // todo: temporary implementation
            carPhysics->mReferenceCar->ReceiveDamage(damageInfo);
            continue;
        }

        if (PedPhysicsBody* pedPhysics = currElement.mPedComponent)
        {
            // todo: temporary implementation
            pedPhysics->mReferencePed->ReceiveDamage(damageInfo);
            continue;
        }
    }

    queryResult.Clear();
}

void Explosion::DisablePrimaryDamage()
{
    mPrimaryDamageDone = true;
}

void Explosion::DisableSecondaryDamage()
{
    mSecondaryDamageDone = true;
}
