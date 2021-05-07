#include "stdafx.h"
#include "Projectile.h"
#include "PhysicsManager.h"
#include "SpriteManager.h"
#include "GameMapManager.h"
#include "TimeManager.h"
#include "DebugRenderer.h"
#include "GameObjectsManager.h"
#include "AudioManager.h"

Projectile::Projectile(WeaponInfo* weaponInfo, Pedestrian* shooter) 
    : GameObject(eGameObjectClass_Projectile, GAMEOBJECT_ID_NULL)
    , mWeaponInfo(weaponInfo)
    , mShooter(shooter)
{
}

void Projectile::HandleSpawn()
{
    debug_assert(mWeaponInfo);

    mStartPosition = mTransform.mPosition;

    PhysicsBody* projectilePhysics = gPhysics.CreateBody(this, PhysicsBodyFlags_Bullet | PhysicsBodyFlags_FixRotation | PhysicsBodyFlags_NoGravity);
    debug_assert(projectilePhysics);

    CollisionShape shapeData;
    shapeData.SetAsCircle(mWeaponInfo ? mWeaponInfo->mProjectileSize : 0.1f);
    PhysicsMaterial shapeMaterial;
    Collider* collisionShape = projectilePhysics->AddCollider(0, shapeData, shapeMaterial, CollisionGroup_Projectile, 
        CollisionGroup_Pedestrian | CollisionGroup_Car | CollisionGroup_Obstacle | CollisionGroup_MapBlock, ColliderFlags_None);
    debug_assert(collisionShape);

    SetPhysics(projectilePhysics);

    // setup animation
    mAnimationState.Clear();

    if (mWeaponInfo)
    {
        StyleData& cityStyle = gGameMap.mStyleData;

        int objectindex = mWeaponInfo->mProjectileObject;
        if (objectindex > 0 && objectindex < (int) cityStyle.mObjects.size())
        {
            GameObjectInfo& objectInfo = cityStyle.mObjects[objectindex];
            debug_assert(objectInfo.mClassID == eGameObjectClass_Projectile);
            
            mAnimationState.mAnimDesc = objectInfo.mAnimationData;

            eSpriteAnimLoop loopType = eSpriteAnimLoop_FromStart;
            if (!mWeaponInfo->IsFireDamage()) // todo: move to configs
            {
                loopType = eSpriteAnimLoop_None;
            }
            mAnimationState.PlayAnimation(loopType, eSpriteAnimMode_Normal, 24.0f); // todo: move to config
            SetupAnimFrameSprite();
        }
    }
    mDrawSprite.mDrawOrder = eSpriteDrawOrder_Projectiles;
    ClearCurrentHit();
}

void Projectile::SimulationStep()
{
    debug_assert(mPhysicsBody);

    if (mHitObject)
    {
        mPhysicsBody->ClearForces(); // stop
        return;
    }

    if (mWeaponInfo == nullptr)
    {
        debug_assert(false);
        return;
    }

    // setup physics
    glm::vec2 velocity = mTransform.GetDirectionVector();
    mPhysicsBody->SetLinearVelocity(velocity * mWeaponInfo->mProjectileSpeed);

    glm::vec2 currPosition = mTransform.GetPosition2();
    glm::vec2 startPosition(mStartPosition.x, mStartPosition.z);

    if (glm::distance(startPosition, currPosition) >= mWeaponInfo->mBaseHitRange)
    {
        MarkForDeletion();
    }

    // inspect current contacts
    for (const Contact& currContact: mObjectsContacts)
    {
        if (!currContact.HasContactPoints())
            continue;

        if (currContact.mThatObject->IsPedestrianClass())
        {
            Pedestrian* otherPedestrian = (Pedestrian*) currContact.mThatObject;
            if (mShooter && (mShooter == otherPedestrian)) // ignore shooter ped
                continue;

            if (otherPedestrian->IsDead() || otherPedestrian->IsAttachedToObject())
                continue;

            if (mWeaponInfo->IsFireDamage() && otherPedestrian->IsBurn())
                continue;
        }

        if (currContact.mThatObject->IsVehicleClass())
        {
            Vehicle* otherCar = (Vehicle*) currContact.mThatObject;
            if (mWeaponInfo->IsFireDamage() && otherCar->IsWrecked())
                continue;
        }

        mHitSomething = true;
        mHitObject = currContact.mThatObject;
        mHitPoint = currContact.mContactPoints[0]; // get first contact point
        break;
    }
}

bool Projectile::ShouldCollide(GameObject* otherObject) const
{
    // disable collision resolving
    return false;
}

void Projectile::HandleCollisionWithMap(const MapCollision& collision)
{
    if (!collision.HasContactPoints())
    {
        debug_assert(false);
        return;
    }

    mHitSomething = true;
    mHitPoint = collision.mContactPoints[0]; // get first contact point
}

void Projectile::UpdateFrame()
{
    if (mWeaponInfo == nullptr)
    {
        MarkForDeletion();
        return;
    }

    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mAnimationState.UpdateFrame(deltaTime))
    {
        SetupAnimFrameSprite();
    }

    if (!mHitSomething)
        return;

    if (mHitObject)
    {
        DamageInfo damageInfo;
        damageInfo.SetDamageFromWeapon(*mWeaponInfo, this);

        mHitObject->ReceiveDamage(damageInfo);
    }

    glm::vec3 hitPosition(mHitPoint.mPosition.x, mHitPoint.mPositionY, mHitPoint.mPosition.y);

    if (mWeaponInfo->IsExplosionDamage())
    {
        if (!mHitObject)
        {
            hitPosition.y += Convert::MapUnitsToMeters(1.0f);
        }

        Explosion* explosion = gGameObjectsManager.CreateExplosion(mHitObject, mShooter, eExplosionType_Rocket, hitPosition);
        debug_assert(explosion);
    }

    if (mWeaponInfo->mProjectileHitEffect > GameObjectType_Null)
    {
        GameObjectInfo& objectInfo = gGameMap.mStyleData.mObjects[mWeaponInfo->mProjectileHitEffect];
        Decoration* hitEffect = gGameObjectsManager.CreateDecoration(hitPosition, cxx::angle_t(), &objectInfo);
        debug_assert(hitEffect);

        if (hitEffect)
        {
            hitEffect->SetDrawOrder(eSpriteDrawOrder_Projectiles);
            hitEffect->SetLifeDuration(1);
        }
    }

    if (mWeaponInfo->mProjectileHitObjectSound != -1)
    {
        StartGameObjectSound(0, eSfxSampleType_Level, mWeaponInfo->mProjectileHitObjectSound, SfxFlags_RandomPitch);
    }
    MarkForDeletion();
}

void Projectile::DebugDraw(DebugRenderer& debugRender)
{   
    if (mWeaponInfo)
    {
        cxx::bounding_sphere_t bsphere (mPhysicsBody->GetPosition(), mWeaponInfo->mProjectileSize);
        debugRender.DrawSphere(bsphere, Color32_Orange, false);
    }
}

void Projectile::SetupAnimFrameSprite()
{
    gSpriteManager.GetSpriteTexture(mObjectID, mAnimationState.GetSpriteIndex(), 0, mDrawSprite);

    // mirror vertically
    std::swap(mDrawSprite.mTextureRegion.mV1, mDrawSprite.mTextureRegion.mV0);

    RefreshDrawSprite();
}

void Projectile::ClearCurrentHit()
{
    mHitObject.reset();
    mHitSomething = false;
}
