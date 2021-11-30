#include "stdafx.h"
#include "Explosion.h"
#include "TimeManager.h"
#include "SpriteManager.h"
#include "DebugRenderer.h"
#include "PhysicsManager.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "BroadcastEventsManager.h"
#include "GameObjectsManager.h"
#include "AudioManager.h"

Explosion::Explosion(GameObject* explodingObject, Pedestrian* causer, eExplosionType explosionType) 
    : GameObject(eGameObjectClass_Explosion, GAMEOBJECT_ID_NULL)
    , mExplosionType(explosionType)
    , mExplodingObject(explodingObject)
    , mExplosionCauser(causer)
{
}

void Explosion::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mAnimationState.UpdateFrame(deltaTime))
    {
        gSpriteManager.GetExplosionTexture(mAnimationState.GetSpriteIndex(), mDrawSprite);
        RefreshDrawSprite();

        if (mAnimationState.mFrameCursor == 6) // todo: magic numbers
        {
            glm::vec3 currentPosition = mTransform.mPosition;
            // create smoke effect
            Decoration* bigSmoke = gGameObjectsManager.CreateBigSmoke(currentPosition);
            debug_assert(bigSmoke);
        }
    }

    if (mUpdatesCounter == 0)
    {
        if (mExplosionType == eExplosionType_Rocket)
        {
            DamageObjectInContact();
        }
        DamageCarsNearby();
        DamagePedsNearby(mExplosionType == eExplosionType_Rocket);
    }

    if (!mAnimationState.IsActive())
    {
        MarkForDeletion();
    }

    ++mUpdatesCounter;
}

void Explosion::DebugDraw(DebugRenderer& debugRender)
{
    float damageRadius = gGameParams.mExplosionRadius;
    debugRender.DrawSphere(mTransform.mPosition, damageRadius, Color32_Red, false);
}

void Explosion::HandleSpawn()
{
    mRemapClut = 0;
    mUpdatesCounter = 0;

    mDrawSprite.mDrawOrder = eSpriteDrawOrder_Explosion;

    mAnimationState.Clear();
    // todo: what should be done here ?
    int numFrames = gSpriteManager.GetExplosionFramesCount();
    mAnimationState.mAnimDesc.SetFrames(0, numFrames);
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
    mAnimationState.SetMaxRepeatCycles(1);

    gSpriteManager.GetExplosionTexture(0, mDrawSprite);
    RefreshDrawSprite();

    // broadcast event
    gBroadcastEvents.ReportEvent(eBroadcastEvent_Explosion, mTransform.GetPosition2(), gGameParams.mBroadcastExplosionEventDuration);

    StartGameObjectSound(0, eSfxSampleType_Level, SfxLevel_HugeExplosion, SfxFlags_RandomPitch);
}

void Explosion::DamagePedsNearby(bool enableInstantKill)
{
    float killHitDistance = gGameParams.mExplosionRadius * 0.5f;
    float killlHitDistance2 = killHitDistance * killHitDistance;
    float burnDistance2 = gGameParams.mExplosionRadius * gGameParams.mExplosionRadius;

    glm::vec2 centerPoint (mTransform.mPosition.x, mTransform.mPosition.z);
    glm::vec2 extents ( 
        gGameParams.mExplosionRadius, 
        gGameParams.mExplosionRadius );

    PhysicsQueryResult queryResult;
    gPhysics.QueryObjectsWithinBox(centerPoint, extents, queryResult, CollisionGroup_Pedestrian);

    for (int icurr = 0; icurr < queryResult.mElementsCount; ++icurr)
    {
        PhysicsQueryElement& currElement = queryResult.mElements[icurr];

        if (currElement.mPhysicsObject == nullptr)
        {
            debug_assert(false);
            continue;
        }

        GameObject* gameObject = currElement.mPhysicsObject->mGameObject;
        if ((gameObject == nullptr) || !gameObject->IsPedestrianClass())
        {
            debug_assert(false);
            continue;
        }

        Pedestrian* currPedestrian = (Pedestrian*) gameObject;
        if (currPedestrian == nullptr)
            continue;

        glm::vec2 pedestrianPosition = currPedestrian->mTransform.GetPosition2();
        float distanceToExplosionCenter2 = glm::distance2(centerPoint, pedestrianPosition);

        if (enableInstantKill)
        {
            if (distanceToExplosionCenter2 < killlHitDistance2)
            {
                // kill instantly
                DamageInfo damageInfo;
                damageInfo.SetExplosionDamage(this);
                currPedestrian->ReceiveDamage(damageInfo);
                continue;
            }
        }

        if (distanceToExplosionCenter2 < burnDistance2)
        {
            // burn
            DamageInfo damageInfo;
            damageInfo.SetFireDamage(this);
            currPedestrian->ReceiveDamage(damageInfo);
            continue;
        }

    }
    queryResult.Clear();
}

void Explosion::DamageObjectInContact()
{
    if (mExplodingObject)
    {
        DamageInfo damageInfo;
        damageInfo.SetExplosionDamage(this);
        mExplodingObject->ReceiveDamage(damageInfo);
    }
}

void Explosion::DamageCarsNearby()
{
    float explodeDistance2 = gGameParams.mExplosionRadius * gGameParams.mExplosionRadius;

    glm::vec2 centerPoint (mTransform.mPosition.x, mTransform.mPosition.z);
    glm::vec2 extents ( 
        gGameParams.mExplosionRadius, 
        gGameParams.mExplosionRadius );

    PhysicsQueryResult queryResult;
    gPhysics.QueryObjectsWithinBox(centerPoint, extents, queryResult, CollisionGroup_Car);

    for (int icurr = 0; icurr < queryResult.mElementsCount; ++icurr)
    {
        PhysicsQueryElement& currElement = queryResult.mElements[icurr];

        if (currElement.mPhysicsObject == nullptr)
        {
            debug_assert(false);
            continue;
        }

        GameObject* gameObject = currElement.mPhysicsObject->mGameObject;
        if ((gameObject == nullptr) || !gameObject->IsVehicleClass())
        {
            debug_assert(false);
            continue;
        }

        Vehicle* currentCar = (Vehicle*) gameObject;
        if (currentCar == mExplodingObject)
            continue;

        glm::vec2 carPosition = currentCar->mTransform.GetPosition2();
        float distanceToExplosionCenter2 = glm::distance2(centerPoint, carPosition);
        if (distanceToExplosionCenter2 < explodeDistance2)
        {
            DamageInfo damageInfo;
            damageInfo.SetExplosionChainDamage(this);
            currentCar->ReceiveDamage(damageInfo);
        }
    }
    queryResult.Clear();
}