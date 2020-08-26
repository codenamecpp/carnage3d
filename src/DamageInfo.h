#pragma once

#include "GameDefs.h"
#include "WeaponInfo.h"

// Game object damage type
enum eDamageCause
{
    eDamageCause_Gravity, // fall from height
    eDamageCause_Electricity, // rails
    eDamageCause_Burning,
    eDamageCause_Drowning,
    eDamageCause_CarCrash,
    eDamageCause_Explosion,
    eDamageCause_Bullet,
    eDamageCause_Punch,
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
    void SetDamageFromCarCrash(const glm::vec3& contactPoint, float contactImpulse, GameObject* object);
    void SetDamageFromExplosion(int hitpoints, GameObject* object);
    void SetDamageFromBullet(int hitpoints, GameObject* object);
    void SetDamageFromPunch(int hitpoints, GameObject* object);

    void Clear();

public:
    // depending on cause of damage object reaction may vary
    eDamageCause mDamageCause = eDamageCause_Punch;

    // has meaning only if car crash
    glm::vec3 mContactPoint;
    glm::vec2 mNormal;

    // object which cause damage, optional
    GameObject* mSourceObject = nullptr;

    float mContactImpulse = 0.0f; // has meaning only if car crash
    float mFallHeight = 0.0f; // has meaning only if fall

    int mHitPoints = 0;
};