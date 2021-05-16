#pragma once

#include "GameDefs.h"

// weapon type description
class WeaponInfo
{
public:
    WeaponInfo() = default;

    // Load weapon info from json node
    bool SetupFromConfg(cxx::json_document_node configNode);

    // Reset weapon info to default state
    void Clear();

    // Get projectile spawn point offset relative pedestrian center
    // @param offset: Output offset, meters
    bool GetProjectileOffsetForAnimation(ePedestrianAnimID animID, glm::vec2& offset) const;

    // Shortcuts
    bool IsMelee() const { return mFireTypeID == eWeaponFireType_Melee; }
    bool IsRange() const { return mFireTypeID == eWeaponFireType_Projectile; }

    // causes death 
    bool IsLethal() const { return mWeaponID != eWeapon_Fists; }

    bool IsBulletDamage() const 
    { 
        return (mFireTypeID == eWeaponFireType_Projectile) && (mProjectileTypeID == eProjectileType_Bullet); 
    }
    // burn rather kill instantly
    bool IsFireDamage() const 
    { 
        return (mFireTypeID == eWeaponFireType_Projectile) && (mProjectileTypeID == eProjectileType_Flame); 
    }
    // explodes when hit
    bool IsExplosionDamage() const 
    { 
        return (mFireTypeID == eWeaponFireType_Projectile) && (mProjectileTypeID == eProjectileType_Missile); 
    }

public:
    eWeaponID mWeaponID = eWeapon_Fists;

    eWeaponFireType mFireTypeID = eWeaponFireType_Melee;
    eProjectileType mProjectileTypeID = eProjectileType_Bullet; // only used if firetype is projectile

    struct ProjectileOffset
    {
        ePedestrianAnimID mAnimationID;
        glm::vec2 mOffset2; // relatipe to pedestrian center, meters
    };
    std::vector<ProjectileOffset> mProjectileOffsets;

    float mBaseHitRange = 1.0f; // how far weapon can damage, for projectiles this is fly distance
    float mBaseFireRate = 1.0f; // num shots per seconds
    float mProjectileSize = 1.0f; // radius of projectile bounding sphere, meters
    float mProjectileSpeed = 1.0f; // how fast projectile moves, meters
    int mShotsPerClip = 1; // how much shots can be done for single ammo point
    int mProjectileHitEffect = GameObjectType_Null;
    int mProjectileHitObjectSound = -1; // level sfx
    int mProjectileObject = GameObjectType_BulletProjectile;
    int mBaseMaxAmmo = 0; // max ammo, 0 is unlimited
    int mShotSound = -1; // level sfx
    int mSpriteIndex = 0;
};