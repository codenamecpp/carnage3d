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

Explosion::Explosion(GameObject* explodingObject, GameObject* causer, eExplosionType explosionType) 
    : GameObject(eGameObjectClass_Explosion, GAMEOBJECT_ID_NULL)
    , mExplosionType(explosionType)
    , mExplodingObject(explodingObject)
    , mExplosionCauser(causer)
{
}

void Explosion::PreDrawFrame()
{
}

void Explosion::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mAnimationState.UpdateFrame(deltaTime))
    {
        gSpriteManager.GetExplosionTexture(mAnimationState.GetSpriteIndex(), mDrawSprite);

        if (mAnimationState.mFrameCursor == 6) // todo: magic numbers
        {
            glm::vec3 currentPosition = GetPosition();
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
    debugRender.DrawSphere(mSpawnPosition, damageRadius, Color32_Red, false);
}

glm::vec3 Explosion::GetPosition() const
{
    return mSpawnPosition;
}

glm::vec2 Explosion::GetPosition2() const
{
    return { mSpawnPosition.x, mSpawnPosition.z };
}

void Explosion::OnGameObjectSpawn()
{
    mUpdatesCounter = 0;

    mDrawSprite.mPosition.x = mSpawnPosition.x;
    mDrawSprite.mPosition.y = mSpawnPosition.z;
    mDrawSprite.mHeight = mSpawnPosition.y;
    mDrawSprite.mDrawOrder = eSpriteDrawOrder_Explosion;

    mAnimationState.Clear();
    // todo
    int numFrames = gSpriteManager.GetExplosionFramesCount();
    mAnimationState.mAnimDesc.SetFrames(0, numFrames);
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
    mAnimationState.SetMaxRepeatCycles(1);

    if (!gSpriteManager.GetExplosionTexture(0, mDrawSprite))
    {
        debug_assert(false);
    }

    // broadcast event
    glm::vec2 position2 (mSpawnPosition.x, mSpawnPosition.z);
    gBroadcastEvents.RegisterEvent(eBroadcastEvent_Explosion, position2, gGameParams.mBroadcastExplosionEventDuration);

    StartGameObjectSound(0, eSfxType_Level, SfxLevel_HugeExplosion, SfxFlags_RandomPitch);
}

void Explosion::DamagePedsNearby(bool enableInstantKill)
{
    float killHitDistance = gGameParams.mExplosionRadius * 0.5f;
    float killlHitDistance2 = killHitDistance * killHitDistance;
    float burnDistance2 = gGameParams.mExplosionRadius * gGameParams.mExplosionRadius;

    glm::vec2 centerPoint (mSpawnPosition.x, mSpawnPosition.z);
    glm::vec2 extents ( 
        gGameParams.mExplosionRadius, 
        gGameParams.mExplosionRadius );

    PhysicsQueryResult queryResult;
    gPhysics.QueryObjectsWithinBox(centerPoint, extents, queryResult);

    for (int icurr = 0; icurr < queryResult.mElementsCount; ++icurr)
    {
        PhysicsQueryElement& currElement = queryResult.mElements[icurr];

        Pedestrian* currPedestrian = nullptr;
        if (PedestrianPhysics* physicsComponent = currElement.mPedComponent)
        {
            currPedestrian = physicsComponent->mReferencePed;
        }

        if (currPedestrian == nullptr)
            continue;

        glm::vec2 pedestrianPosition = currPedestrian->GetPosition2();
        float distanceToExplosionCenter2 = glm::distance2(centerPoint, pedestrianPosition);

        if (enableInstantKill)
        {
            if (distanceToExplosionCenter2 < killlHitDistance2)
            {
                // kill instantly
                DamageInfo damageInfo;
                damageInfo.SetDamageFromExplosion(100, this); // max hitpoints
                currPedestrian->ReceiveDamage(damageInfo);
                continue;
            }
        }

        if (distanceToExplosionCenter2 < burnDistance2)
        {
            // burn
            DamageInfo damageInfo;
            damageInfo.SetDamageFromFire(1, this);
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
        damageInfo.SetDamageFromExplosion(100, this); // max hitpoints, explode instantly
        mExplodingObject->ReceiveDamage(damageInfo);
    }
}

void Explosion::DamageCarsNearby()
{
    float explodeDistance2 = gGameParams.mExplosionRadius * gGameParams.mExplosionRadius;

    glm::vec2 centerPoint (mSpawnPosition.x, mSpawnPosition.z);
    glm::vec2 extents ( 
        gGameParams.mExplosionRadius, 
        gGameParams.mExplosionRadius );

    PhysicsQueryResult queryResult;
    gPhysics.QueryObjectsWithinBox(centerPoint, extents, queryResult);

    for (int icurr = 0; icurr < queryResult.mElementsCount; ++icurr)
    {
        PhysicsQueryElement& currElement = queryResult.mElements[icurr];

        Vehicle* currentCar = nullptr;
        if (CarPhysics* physicsComponent = currElement.mCarComponent)
        {
            currentCar = physicsComponent->mReferenceCar;
        }

        if ((currentCar == nullptr) || (currentCar == mExplodingObject))
            continue;

        glm::vec2 carPosition = currentCar->GetPosition2();
        float distanceToExplosionCenter2 = glm::distance2(centerPoint, carPosition);
        if (distanceToExplosionCenter2 < explodeDistance2)
        {
            DamageInfo damageInfo;
            damageInfo.SetDamageFromExplosion(1, this); // explode with delay
            currentCar->ReceiveDamage(damageInfo);
        }
    }
    queryResult.Clear();
}