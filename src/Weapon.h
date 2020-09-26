#pragma once

#include "WeaponInfo.h"

// Class stores all weapon related states info
class Weapon final
{
public:
    // readonly
    eWeaponState mWeaponState = eWeaponState_OutOfAmmunition;
    eWeaponID mWeaponID = eWeapon_Fists;
    
    float mLastFireTime = 0.0f;
    int mAmmunition = 0;
    int mShotsCounter = 0;

public:
    Weapon() = default;
    WeaponInfo* GetWeaponInfo() const;

    // One time weapon initialization
    void Setup(eWeaponID weaponID, int ammunition);
    void UpdateFrame();
    void ClearLastFireTime();
    void ClearShotsCounter();

    // Try to make shot from weapon, depending on current state it might fail
    bool Fire(Pedestrian* attacker);

    // Change weapons current ammunition amount
    // @param ammunition: New amount
    void SetAmmunition(int ammunition);
    void AddAmmunition(int ammunition);
    void DecAmmunition(int ammunition);

    bool IsUsesAmmunition() const;

    // Current weapon state shortcuts
    bool IsOutOfAmmunition() const;
    bool IsReadyToFire() const;
    bool IsReloading() const;

private:
    bool IsReloadingTime() const;
};
