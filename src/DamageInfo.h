#pragma once

#include "GameDefs.h"
#include "WeaponInfo.h"
#include "Collision.h"

// Game object damage type
enum eDamageCause
{
    eDamageCause_Gravity, // fall from height
    eDamageCause_Electricity, // rails
    eDamageCause_Flame,
    eDamageCause_Water,
    eDamageCause_Collision, // physics contact between two object
    eDamageCause_MapCollision, // physics contact with map blocks or walls
    eDamageCause_Explosion,
    eDamageCause_ExplosionChain,
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
    void SetFallDamage(float fallHeight);
    void SetElectricityDamage();
    void SetFireDamage(GameObject* object);
    void SetDamage(const WeaponInfo& weaponInfo, GameObject* object);
    void SetWaterDamage();
    void SetCollisionDamage(const Collision& collisionInfo);
    void SetCollisionDamage(const MapCollision& collisionInfo);
    void SetExplosionDamage(GameObject* object);
    void SetExplosionChainDamage(GameObject* object);
    void SetBulletDamage(GameObject* object);
    void SetPunchDamage(GameObject* object);
    void SetCarHitDamage(GameObject* carObject);

    void Clear();

    Pedestrian* GetDamageCauser() const;

public:
    // depending on cause of damage object reaction may vary
    eDamageCause mDamageCause = eDamageCause_Punch;

    // object which cause damage, optional
    GameObject* mSourceObject = nullptr;

    // collision specific data
    ContactPoint mContactPoint;
    float mContactImpulse = 0.0f;
    float mFallHeight = 0.0f; // has meaning only if fall
};