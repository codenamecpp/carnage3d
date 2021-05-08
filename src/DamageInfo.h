#pragma once

#include "GameDefs.h"
#include "WeaponInfo.h"
#include "Collision.h"

// Game object damage type
enum eDamageCause
{
    eDamageCause_Gravity, // fall from height
    eDamageCause_Electricity, // rails
    eDamageCause_Burning,
    eDamageCause_Drowning,
    eDamageCause_Collision, // physics contact between two object
    eDamageCause_MapCollision, // physics contact with map blocks or walls
    eDamageCause_Explosion,
    eDamageCause_Bullet,
    eDamageCause_Punch,
    eDamageCause_CarHit,
};

// Game object damage information
struct DamageInfo
{
public:
    DamageInfo() = default;

    // @param object: Source object, it is optional
    void SetDamageFromFall(float fallHeight);
    void SetDamageFromElectricity();
    void SetDamageFromFire(int hitpoints, GameObject* object);
    void SetDamageFromWeapon(const WeaponInfo& weaponInfo, GameObject* object);
    void SetDamageFromWater(int hitpoints);
    void SetDamageFromCollision(const Collision& collisionInfo);
    void SetDamageFromCollision(const MapCollision& collisionInfo);
    void SetDamageFromExplosion(int hitpoints, GameObject* object);
    void SetDamageFromBullet(int hitpoints, GameObject* object);
    void SetDamageFromPunch(int hitpoints, GameObject* object);
    void SetDamageFromCarHit(GameObject* carObject);

    void Clear();

public:
    // depending on cause of damage object reaction may vary
    eDamageCause mDamageCause = eDamageCause_Punch;

    // collision specific data
    ContactPoint mContactPoint;
    float mContactImpulse = 0.0f;

    // object which cause damage, optional
    GameObject* mSourceObject = nullptr;

    float mFallHeight = 0.0f; // has meaning only if fall
    int mHitPoints = 0;
};