#include "stdafx.h"
#include "WeaponInfo.h"

bool WeaponInfo::SetupFromConfg(cxx::json_document_node configNode)
{
    Clear();

    if (!cxx::json_get_attribute(configNode, "weapon_type", mWeaponID))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Unknown weapon type ID");
        return false;
    }
    if (!cxx::json_get_attribute(configNode, "fire_type", mFireTypeID))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Unknown weapon fire type ID");
        return false;
    }
    // read projectile data
    if (mFireTypeID == eWeaponFireType_Projectile)
    {
        if (!cxx::json_get_attribute(configNode, "projectile_type", mProjectileTypeID))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Unknown weapon projectile type ID");
            return false;
        }
        // conver map units to meters
        if (cxx::json_get_attribute(configNode, "projectile_size", mProjectileSize))
        {
            mProjectileSize = Convert::MapUnitsToMeters(mProjectileSize);
        }
        // conver map units to meters
        if (cxx::json_get_attribute(configNode, "projectile_speed", mProjectileSpeed))
        {
            mProjectileSpeed = Convert::MapUnitsToMeters(mProjectileSpeed);
        }
        cxx::json_get_attribute(configNode, "projectile_hit_effect", mProjectileHitEffect);
        cxx::json_get_attribute(configNode, "projectile_object", mProjectileObject);
    }
    // conver map units to meters
    if (cxx::json_get_attribute(configNode, "base_hit_range", mBaseHitRange))
    {
        mBaseHitRange = Convert::MapUnitsToMeters(mBaseHitRange);
    }
    cxx::json_get_attribute(configNode, "base_fire_rate", mBaseFireRate);
    cxx::json_get_attribute(configNode, "base_ammo_limit", mBaseMaxAmmo);
    cxx::json_get_attribute(configNode, "hud_sprite", mSpriteIndex);
    return true;
}

void WeaponInfo::Clear()
{
    mWeaponID = eWeapon_Fists;

    mFireTypeID = eWeaponFireType_Melee;
    mProjectileTypeID = eProjectileType_Bullet;

    mBaseHitRange = 1.0f;
    mBaseFireRate = 1.0f;
    mProjectileSize = 1.0f;
    mProjectileSpeed = 1.0f; 

    mProjectileObject = GameObjectType_BulletProjectile;
    mBaseMaxAmmo = 0;
    mSpriteIndex = 0;
}