#include "stdafx.h"
#include "DamageInfo.h"
#include "Collision.h"
#include "GameObjectHelpers.h"

void DamageInfo::SetFallDamage(float fallHeight)
{
    mDamageCause = eDamageCause_Gravity;
    mFallHeight = fallHeight;
    // no hitpoints damage
}

void DamageInfo::SetElectricityDamage()
{
    mDamageCause = eDamageCause_Electricity;
    // no hitpoints damage
}

void DamageInfo::SetFireDamage(GameObject* object)
{
    mDamageCause = eDamageCause_Flame;
    mSourceObject = object;
}

void DamageInfo::SetDamage(const WeaponInfo& weaponInfo, GameObject* object)
{    
    if (weaponInfo.IsBulletDamage())
    {
        SetBulletDamage(object);
        return;
    }

    if (weaponInfo.IsFireDamage())
    {
        SetFireDamage(object);
        return;
    }

    if (weaponInfo.IsMelee())
    {
        SetPunchDamage(object);
        return;
    }

    if (weaponInfo.IsExplosionDamage())
    {
        SetExplosionDamage(object);
        return;
    }

    debug_assert(false);
    // unknown damage type
}

void DamageInfo::SetWaterDamage()
{
    mDamageCause = eDamageCause_Water;
}

void DamageInfo::SetCollisionDamage(const Collision& collisionInfo)
{
    mDamageCause = eDamageCause_Collision;
    mContactImpulse = collisionInfo.GetContactImpulse();
    if (!collisionInfo.mContactInfo.HasContactPoints())
    {
        debug_assert(false);
        return;
    }
    mContactPoint = collisionInfo.mContactInfo.mContactPoints[0];
    mSourceObject = collisionInfo.mContactInfo.mThatObject;
    debug_assert(mSourceObject);
}

void DamageInfo::SetCollisionDamage(const MapCollision& collisionInfo)
{
    mDamageCause = eDamageCause_MapCollision;
    mContactImpulse = collisionInfo.GetContactImpulse();
    if (!collisionInfo.HasContactPoints())
        return;

    mContactPoint = collisionInfo.mContactPoints[0];
}

void DamageInfo::SetExplosionDamage(GameObject* object)
{
    mDamageCause = eDamageCause_Explosion;
    mSourceObject = object;
}

void DamageInfo::SetExplosionChainDamage(GameObject* object)
{
    mDamageCause = eDamageCause_ExplosionChain;
    mSourceObject = object;
}

void DamageInfo::SetBulletDamage(GameObject* object)
{
    mDamageCause = eDamageCause_Bullet;
    mSourceObject = object;
}

void DamageInfo::SetPunchDamage(GameObject* object)
{
    mDamageCause = eDamageCause_Punch;
    mSourceObject = object;
}

void DamageInfo::SetCarHitDamage(GameObject* carObject)
{
    mDamageCause = eDamageCause_CarHit;
    mSourceObject = carObject;
}

void DamageInfo::Clear()
{
    mDamageCause = eDamageCause_Punch;
    mSourceObject = nullptr;
    mContactImpulse = 0.0f;
    mFallHeight = 0.0f;
}

Pedestrian* DamageInfo::GetDamageCauser() const
{
    if (mSourceObject)
    {
        if (Pedestrian* pedestrian = ToPedestrian(mSourceObject))
        {
            return pedestrian;
        }

        if (Vehicle* carObject = ToVehicle(mSourceObject))
        {
            return carObject->GetCarDriver();
        }

        if (Explosion* explosionObjet = ToExplosion(mSourceObject))
        {
            return explosionObjet->mExplosionCauser;
        }

        if (Projectile* projectileObject = ToProjectile(mSourceObject))
        {
            return projectileObject->mShooter;
        }
    }
    return nullptr;
}
