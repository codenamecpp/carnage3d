#include "stdafx.h"
#include "DamageInfo.h"

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

void DamageInfo::SetDamageFromCarCrash(const glm::vec3& contactPoint, float contactImpulse, GameObject* object)
{
    mDamageCause = eDamageCause_CarCrash;
    mContactPoint = contactPoint;
    mContactImpulse = contactImpulse;
    mSourceObject = object;
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

void DamageInfo::Clear()
{
    mDamageCause = eDamageCause_Punch;

    mContactPoint.x = 0.0f;
    mContactPoint.y = 0.0f;
    mContactPoint.z = 0.0f;
    mNormal.x = 0.0f;
    mNormal.y = 0.0f;

    mSourceObject = nullptr;
    mHitPoints = 0;
    mContactImpulse = 0.0f;
    mFallHeight = 0.0f;
}