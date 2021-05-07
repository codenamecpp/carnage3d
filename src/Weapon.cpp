#include "stdafx.h"
#include "Weapon.h"
#include "GameMapManager.h"
#include "TimeManager.h"
#include "BroadcastEventsManager.h"
#include "GameObjectsManager.h"
#include "PhysicsManager.h"
#include "AudioManager.h"
#include "GameObjectHelpers.h"

void Weapon::Setup(eWeaponID weaponID, int ammunition)
{
    mWeaponID = weaponID;
    mLastFireTime = 0.0f;
    mShotsCounter = 0;
    mWeaponState = eWeaponState_ReadyToFire;
    SetAmmunition(ammunition);
}

void Weapon::UpdateFrame()
{
    // update reloading state
    if (mWeaponState == eWeaponState_Reloading)
    {
        if (!IsReloadingTime())
        {
            mWeaponState = eWeaponState_ReadyToFire;
        }
    }
}

bool Weapon::Fire(Pedestrian* shooter)
{
    if (!IsReadyToFire())
        return false;

    debug_assert(shooter);
    glm::vec3 currPosition = shooter->mTransform.mPosition;

    WeaponInfo* weaponInfo = GetWeaponInfo();
    if (weaponInfo->IsMelee())
    {
        glm::vec2 posA { currPosition.x, currPosition.z };
        glm::vec2 posB = posA + (shooter->mTransform.GetDirectionVector() * weaponInfo->mBaseHitRange);
        // find candidates
        PhysicsQueryResult queryResults;
        gPhysics.QueryObjectsLinecast(posA, posB, queryResults, CollisionGroup_Pedestrian);
        for (int icurr = 0; icurr < queryResults.mElementsCount; ++icurr)
        {
            GameObject* currGameObject = queryResults.mElements[icurr].mPhysicsObject->mGameObject;
            debug_assert(currGameObject);

            Pedestrian* otherPedestrian = ToPedestrian(currGameObject);
            debug_assert(otherPedestrian);

            if (otherPedestrian == shooter) // ignore self
                continue;

            // todo: check distance in y direction
            DamageInfo damageInfo;
            damageInfo.SetDamageFromWeapon(*weaponInfo, shooter);         
            otherPedestrian->ReceiveDamage(damageInfo);
        }
    }
    else if (weaponInfo->IsRange())
    {   
        glm::vec3 projectilePos = currPosition;
        glm::vec2 offset;
        if (weaponInfo->GetProjectileOffsetForAnimation(shooter->GetCurrentAnimationID(), offset))
        {
            offset = shooter->mTransform.GetPoint2(offset, eTransformSpace_World);
            projectilePos.x = offset.x;
            projectilePos.z = offset.y;
        }
        else
        {
            float defaultOffsetLength = gGameParams.mPedestrianBoundsSphereRadius + weaponInfo->mProjectileSize;
            offset = shooter->mTransform.GetDirectionVector() * defaultOffsetLength;
            projectilePos.x += offset.x; 
            projectilePos.z += offset.y;
        }

        debug_assert(weaponInfo->mProjectileTypeID < eProjectileType_COUNT);
        Projectile* projectile = gGameObjectsManager.CreateProjectile(projectilePos, shooter->mTransform.mOrientation, weaponInfo, shooter);
        debug_assert(projectile);

        if (weaponInfo->mShotSound != -1)
        {
            shooter->StartGameObjectSound(ePedSfxChannelIndex_Weapon, eSfxSampleType_Level, weaponInfo->mShotSound, SfxFlags_RandomPitch);
        }

        // broardcast event
        gBroadcastEvents.RegisterEvent(eBroadcastEvent_GunShot, shooter, shooter, gGameParams.mBroadcastGunShotEventDuration);
    }
    else
    {
        debug_assert(false);
    }

    mLastFireTime = gTimeManager.mGameTime;
    ++mShotsCounter;
    if (IsUsesAmmunition())
    {
        if (mShotsCounter >= weaponInfo->mShotsPerClip)
        {
            DecAmmunition(1);
            ClearShotsCounter();
        }
    }

    if (!IsOutOfAmmunition())
    {
        mWeaponState = eWeaponState_Reloading;
    }
    return true;
}

WeaponInfo* Weapon::GetWeaponInfo() const
{
    debug_assert(mWeaponID < eWeapon_COUNT);

    return &gGameMap.mStyleData.mWeaponTypes[mWeaponID];
}

bool Weapon::IsOutOfAmmunition() const
{
    return mWeaponState == eWeaponState_OutOfAmmunition;
}

bool Weapon::IsReadyToFire() const
{
    return mWeaponState == eWeaponState_ReadyToFire;
}

bool Weapon::IsReloading() const
{
    return mWeaponState == eWeaponState_Reloading;
}

void Weapon::SetAmmunition(int ammunition)
{
    if (!IsUsesAmmunition())
        return;

    WeaponInfo* weaponInfo = GetWeaponInfo();
    mAmmunition = glm::clamp(ammunition, 0, weaponInfo->mBaseMaxAmmo);
    // update current out of ammunition state
    if ((mWeaponState == eWeaponState_OutOfAmmunition) && (mAmmunition > 0))
    {
        mWeaponState = IsReloadingTime() ? 
            eWeaponState_Reloading : 
            eWeaponState_ReadyToFire;
    }
    else if (mAmmunition == 0)
    {
        mWeaponState = eWeaponState_OutOfAmmunition;
    }
}

void Weapon::AddAmmunition(int ammunition)
{
    SetAmmunition(mAmmunition + ammunition);
}

void Weapon::DecAmmunition(int ammunition)
{
    SetAmmunition(mAmmunition - ammunition);
}

void Weapon::ClearLastFireTime()
{
    mLastFireTime = 0.0f;
}

void Weapon::ClearShotsCounter()
{
    mShotsCounter = 0;
}

bool Weapon::IsReloadingTime() const
{
    if (mLastFireTime > 0.0f)
    {
        WeaponInfo* weaponInfo = GetWeaponInfo();
        float nextFireTime = mLastFireTime + (1.0f / weaponInfo->mBaseFireRate);
        if (nextFireTime > gTimeManager.mGameTime)
            return true;
    }
    return false;
}

bool Weapon::IsUsesAmmunition() const
{
    WeaponInfo* weaponInfo = GetWeaponInfo();
    return weaponInfo->mBaseMaxAmmo > 0;
}
