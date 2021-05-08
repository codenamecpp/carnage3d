#include "stdafx.h"
#include "DamageInfo.h"
#include "Collision.h"

void DamageInfo::SetDamageFromFall(float fallHeight)
{
    mDamageCause = eDamageCause_Gravity;
    mFallHeight = fallHeight;
    // no hitpoints damage
}

void DamageInfo::SetDamageFromElectricity()
{
    mDamageCause = eDamageCause_Electricity;
    // no hitpoints damage
}

void DamageInfo::SetDamageFromFire(int hitpoints, GameObject* object)
{
    mDamageCause = eDamageCause_Burning;
    mHitPoints = hitpoints;
    mSourceObject = object;
}

void DamageInfo::SetDamageFromWeapon(const WeaponInfo& weaponInfo, GameObject* object)
{
    int hitpoints = weaponInfo.mBaseDamage;
    
    if (weaponInfo.IsBulletDamage())
    {
        SetDamageFromBullet(hitpoints, object);
        return;
    }

    if (weaponInfo.IsFireDamage())
    {
        SetDamageFromFire(hitpoints, object);
        return;
    }

    if (weaponInfo.IsMelee())
    {
        SetDamageFromPunch(hitpoints, object);
        return;
    }

    if (weaponInfo.IsExplosionDamage())
    {
        SetDamageFromExplosion(hitpoints, object);
        return;
    }

    debug_assert(false);
    // unknown damage type
}

void DamageInfo::SetDamageFromWater(int hitpoints)
{
    mDamageCause = eDamageCause_Drowning;
    mHitPoints = hitpoints;
}

void DamageInfo::SetDamageFromCollision(const Collision& collisionInfo)
{
    mDamageCause = eDamageCause_Collision;
    mHitPoints = 0; // not used
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

void DamageInfo::SetDamageFromCollision(const MapCollision& collisionInfo)
{
    mDamageCause = eDamageCause_MapCollision;
    mHitPoints = 0; // not used
    mContactImpulse = collisionInfo.GetContactImpulse();
    if (!collisionInfo.HasContactPoints())
    {
        debug_assert(false);
        return;
    }
    mContactPoint = collisionInfo.mContactPoints[0];
}

void DamageInfo::SetDamageFromExplosion(int hitpoints, GameObject* object)
{
    mDamageCause = eDamageCause_Explosion;
    mHitPoints = hitpoints;
    mSourceObject = object;
}

void DamageInfo::SetDamageFromBullet(int hitpoints, GameObject* object)
{
    mDamageCause = eDamageCause_Bullet;
    mHitPoints = hitpoints;
    mSourceObject = object;
}

void DamageInfo::SetDamageFromPunch(int hitpoints, GameObject* object)
{
    mDamageCause = eDamageCause_Punch;
    mHitPoints = hitpoints;
    mSourceObject = object;
}

void DamageInfo::SetDamageFromCarHit(GameObject* carObject)
{
    mDamageCause = eDamageCause_CarHit;
    mSourceObject = carObject;
    mHitPoints = 0; // not used
}

void DamageInfo::Clear()
{
    mDamageCause = eDamageCause_Punch;
    mSourceObject = nullptr;
    mHitPoints = 0;
    mContactImpulse = 0.0f;
    mFallHeight = 0.0f;
}