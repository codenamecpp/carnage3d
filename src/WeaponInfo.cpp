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
        cxx::json_get_attribute(configNode, "projectile_hit_object_sfx", mProjectileHitObjectSound);

        // read projectile offsets
        if (cxx::json_node_array offsetsNode = configNode["projectile_offset"])
        {
            for (cxx::json_node_object currOffsetNode = offsetsNode.first_child();
                currOffsetNode; currOffsetNode = currOffsetNode.next_sibling())
            {
                ProjectileOffset projectileOffset;
                if (cxx::json_get_attribute(currOffsetNode, "anim", projectileOffset.mAnimationID))
                {
                    if (cxx::json_node_array pxNode = currOffsetNode["px"])
                    {
                        int pixels_x = 0;
                        int pixels_y = 0;
                        cxx::json_get_attribute(pxNode, 0, pixels_x);
                        cxx::json_get_attribute(pxNode, 1, pixels_y);

                        projectileOffset.mOffset2.x = Convert::PixelsToMeters(pixels_x);
                        projectileOffset.mOffset2.y = Convert::PixelsToMeters(pixels_y);
                    }

                    mProjectileOffsets.push_back(projectileOffset);
                }
                else
                {
                    debug_assert(false);
                }
            }
        }
    }
    // conver map units to meters
    if (cxx::json_get_attribute(configNode, "base_hit_range", mBaseHitRange))
    {
        mBaseHitRange = Convert::MapUnitsToMeters(mBaseHitRange);
    }
    cxx::json_get_attribute(configNode, "base_fire_rate", mBaseFireRate);
    cxx::json_get_attribute(configNode, "base_ammo_limit", mBaseMaxAmmo);
    cxx::json_get_attribute(configNode, "hud_sprite", mSpriteIndex);
    cxx::json_get_attribute(configNode, "base_damage", mBaseDamage);
    cxx::json_get_attribute(configNode, "shot_sfx", mShotSound);
    debug_assert(mBaseDamage >= 0);
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
    mProjectileHitObjectSound = -1;
    mProjectileObject = GameObjectType_BulletProjectile;
    mBaseMaxAmmo = 0;
    mSpriteIndex = 0;
    mShotSound = 0;
    mProjectileOffsets.clear();
}

bool WeaponInfo::GetProjectileOffsetForAnimation(ePedestrianAnimID animID, glm::vec2& offset) const
{
    for (const ProjectileOffset& currOffset: mProjectileOffsets)
    {
        if (animID == currOffset.mAnimationID)
        {
            offset = currOffset.mOffset2;
            return true;
        }
    }
    return false;
}
