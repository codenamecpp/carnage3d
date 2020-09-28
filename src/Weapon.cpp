#include "stdafx.h"
#include "Weapon.h"
#include "GameMapManager.h"
#include "TimeManager.h"
#include "BroadcastEventsManager.h"
#include "GameObjectsManager.h"
#include "PhysicsManager.h"
#include "AudioManager.h"

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

bool Weapon::Fire(Pedestrian* attacker)
{
    if (!IsReadyToFire())
        return false;

    debug_assert(attacker);
    glm::vec3 currPosition = attacker->mPhysicsBody->GetPosition();

    WeaponInfo* weaponInfo = GetWeaponInfo();
    if (weaponInfo->IsMelee())
    {
        glm::vec2 posA { currPosition.x, currPosition.z };
        glm::vec2 posB = posA + (attacker->mPhysicsBody->GetSignVector() * weaponInfo->mBaseHitRange);
        // find candidates
        PhysicsLinecastResult linecastResult;
        gPhysics.QueryObjectsLinecast(posA, posB, linecastResult);
        for (int icurr = 0; icurr < linecastResult.mHitsCount; ++icurr)
        {
            PedPhysicsBody* pedBody = linecastResult.mHits[icurr].mPedComponent;
            if (pedBody == nullptr || pedBody->mReferencePed == attacker) // ignore self
                continue; 

            // todo: check distance in y direction
            DamageInfo damageInfo;
            damageInfo.SetDamageFromWeapon(*weaponInfo, attacker);         
            pedBody->mReferencePed->ReceiveDamage(damageInfo);
        }
    }
    else if (weaponInfo->IsRange())
    {
        glm::vec2 signVector = attacker->mPhysicsBody->GetSignVector();       
        glm::vec2 offset = (signVector * 1.0f); //todo: magic numbers
        glm::vec3 projectilePos {
            currPosition.x + offset.x, 
            currPosition.y, 
            currPosition.z + offset.y
        };
        debug_assert(weaponInfo->mProjectileTypeID < eProjectileType_COUNT);
        Projectile* projectile = gGameObjectsManager.CreateProjectile(projectilePos, attacker->mPhysicsBody->GetRotationAngle(), weaponInfo);
        debug_assert(projectile);

        if (weaponInfo->mShotSound != -1)
        {
            gAudioManager.PlaySfxLevel(weaponInfo->mShotSound, projectilePos, false);
        }

        // broardcast event
        gBroadcastEvents.RegisterEvent(eBroadcastEvent_GunShot, attacker, attacker, gGameParams.mBroadcastGunShotEventDuration);
    }
    else
    {
        debug_assert(false);
    }

    mLastFireTime = gTimeManager.mGameTime;
    ++mShotsCounter;
    if (IsUsesAmmunition())
    {
        DecAmmunition(1);
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
