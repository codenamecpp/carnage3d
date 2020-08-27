#include "stdafx.h"
#include "PedestrianStates.h"
#include "Pedestrian.h"
#include "PhysicsComponents.h"
#include "Vehicle.h"
#include "CarnageGame.h"
#include "PhysicsManager.h"
#include "TimeManager.h"

PedestrianStatesManager::PedestrianStatesManager(Pedestrian* pedestrian)
    : mPedestrian(pedestrian)
{
    debug_assert(mPedestrian);
    InitFuncsTable();
}

void PedestrianStatesManager::ChangeState(ePedestrianState nextState, const PedestrianStateEvent& evData)
{
    if (nextState == mCurrentStateID)
        return;

    mPedestrian->mCurrentStateTime = 0;
    debug_assert(nextState > ePedestrianState_Unspecified && nextState < ePedestrianState_COUNT);
    // process exit current state
    (this->*mFuncsTable[mCurrentStateID].pfStateExit)();
    mCurrentStateID = nextState;
    // process enter next state
    (this->*mFuncsTable[mCurrentStateID].pfStateEnter)(evData);
}

bool PedestrianStatesManager::ProcessEvent(const PedestrianStateEvent& evData)
{
    return (this->*mFuncsTable[mCurrentStateID].pfStateEvent)(evData);
}

void PedestrianStatesManager::ProcessFrame()
{
    (this->*mFuncsTable[mCurrentStateID].pfStateFrame)();
}

void PedestrianStatesManager::InitFuncsTable()
{
    mFuncsTable[ePedestrianState_Unspecified] = {&PedestrianStatesManager::StateDummy_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateDummy_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent};

    mFuncsTable[ePedestrianState_StandingStill] = {&PedestrianStatesManager::StateIdle_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateIdle_ProcessFrame, 
        &PedestrianStatesManager::StateIdle_ProcessEvent};

    mFuncsTable[ePedestrianState_StandsAndShoots] = {&PedestrianStatesManager::StateIdle_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateIdle_ProcessFrame, 
        &PedestrianStatesManager::StateIdle_ProcessEvent};

    mFuncsTable[ePedestrianState_Walks] = {&PedestrianStatesManager::StateIdle_ProcessEnter,
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateIdle_ProcessFrame, 
        &PedestrianStatesManager::StateIdle_ProcessEvent};

    mFuncsTable[ePedestrianState_Runs] = {&PedestrianStatesManager::StateIdle_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit,
        &PedestrianStatesManager::StateIdle_ProcessFrame, 
        &PedestrianStatesManager::StateIdle_ProcessEvent};

    mFuncsTable[ePedestrianState_WalksAndShoots] = {&PedestrianStatesManager::StateIdle_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateIdle_ProcessFrame, 
        &PedestrianStatesManager::StateIdle_ProcessEvent};

    mFuncsTable[ePedestrianState_RunsAndShoots] = {&PedestrianStatesManager::StateIdle_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateIdle_ProcessFrame, 
        &PedestrianStatesManager::StateIdle_ProcessEvent};

    mFuncsTable[ePedestrianState_Falling] = {&PedestrianStatesManager::StateFalling_ProcessEnter, 
        &PedestrianStatesManager::StateFalling_ProcessExit, 
        &PedestrianStatesManager::StateDummy_ProcessFrame, 
        &PedestrianStatesManager::StateFalling_ProcessEvent};

    mFuncsTable[ePedestrianState_EnteringCar] = {&PedestrianStatesManager::StateEnterCar_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateEnterCar_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent};

    mFuncsTable[ePedestrianState_ExitingCar] =  {&PedestrianStatesManager::StateExitCar_ProcessEnter, 
        &PedestrianStatesManager::StateExitCar_ProcessExit, 
        &PedestrianStatesManager::StateExitCar_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent};

    mFuncsTable[ePedestrianState_DrivingCar] = {&PedestrianStatesManager::StateDriveCar_ProcessEnter, 
        &PedestrianStatesManager::StateDriveCar_ProcessExit, 
        &PedestrianStatesManager::StateDummy_ProcessFrame, 
        &PedestrianStatesManager::StateDriveCar_ProcessEvent};

    mFuncsTable[ePedestrianState_SlideOnCar] = {&PedestrianStatesManager::StateSlideCar_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateSlideCar_ProcessFrame, 
        &PedestrianStatesManager::StateSlideCar_ProcessEvent};

    mFuncsTable[ePedestrianState_Dead] = {&PedestrianStatesManager::StateDead_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateDummy_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent};

    mFuncsTable[ePedestrianState_KnockedDown] = {&PedestrianStatesManager::StateKnockedDown_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateKnockedDown_ProcessFrame, 
        &PedestrianStatesManager::StateKnockedDown_ProcessEvent};

    mFuncsTable[ePedestrianState_Drowning] = {&PedestrianStatesManager::StateDrowning_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateDrowning_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent};

    mFuncsTable[ePedestrianState_Dies] = {&PedestrianStatesManager::StateDies_ProcessEnter, 
        &PedestrianStatesManager::StateDies_ProcessExit, 
        &PedestrianStatesManager::StateDies_ProcessFrame, 
        &PedestrianStatesManager::StateDies_ProcessEvent};
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::ProcessRotateActions()
{
    const PedestrianCtlState& ctlState = mPedestrian->mCtlState;
    if (ctlState.mCtlActions[ePedestrianAction_TurnLeft] || 
        ctlState.mCtlActions[ePedestrianAction_TurnRight])
    {
        float turnSpeed = gGameParams.mPedestrianTurnSpeed;
        if (mCurrentStateID == ePedestrianState_SlideOnCar)
        {
            turnSpeed = gGameParams.mPedestrianTurnSpeedSlideOnCar;
        }

        cxx::angle_t angularVelocity = cxx::angle_t::from_degrees(turnSpeed * (ctlState.mCtlActions[ePedestrianAction_TurnLeft] ? -1.0f : 1.0f));
        mPedestrian->mPhysicsBody->SetAngularVelocity(angularVelocity);
    }
    else
    {
        cxx::angle_t angularVelocity;
        mPedestrian->mPhysicsBody->SetAngularVelocity(angularVelocity);
    }
}

void PedestrianStatesManager::ProcessMotionActions()
{
    const PedestrianCtlState& ctlState = mPedestrian->mCtlState;

    // while slding on car
    if (mCurrentStateID == ePedestrianState_SlideOnCar)
    {
        glm::vec2 linearVelocity = gGameParams.mPedestrianSlideOnCarSpeed * mPedestrian->mPhysicsBody->GetSignVector();
        mPedestrian->mPhysicsBody->SetLinearVelocity(linearVelocity);
        return;
    }

    glm::vec2 linearVelocity {};
    // generic case
    if (ctlState.mCtlActions[ePedestrianAction_WalkForward] || 
        ctlState.mCtlActions[ePedestrianAction_WalkBackward] || 
        ctlState.mCtlActions[ePedestrianAction_Run])
    {
        float moveSpeed = gGameParams.mPedestrianWalkSpeed;

        linearVelocity = mPedestrian->mPhysicsBody->GetSignVector();
        if (ctlState.mCtlActions[ePedestrianAction_Run])
        {
            moveSpeed = gGameParams.mPedestrianRunSpeed;
        }
        else if (ctlState.mCtlActions[ePedestrianAction_WalkBackward])
        {
            linearVelocity = -linearVelocity;
        }

        linearVelocity *= moveSpeed;
    }
    else
    {
        // force stop
    }
    mPedestrian->mPhysicsBody->SetLinearVelocity(linearVelocity); 
}

bool PedestrianStatesManager::TryToShoot()
{
    debug_assert(mPedestrian->mCurrentWeapon < eWeapon_COUNT);

    float currGameTime = gTimeManager.mGameTime;
    if (mPedestrian->mWeaponRechargeTime >= currGameTime ||
        mPedestrian->mWeaponsAmmo[mPedestrian->mCurrentWeapon] == 0)
    {
        return false;
    }

    glm::vec3 currPosition = mPedestrian->mPhysicsBody->GetPosition();

    // get weapon params
    WeaponInfo& weaponParams = gGameMap.mStyleData.mWeapons[mPedestrian->mCurrentWeapon];

    if (weaponParams.IsMelee())
    {
        glm::vec2 posA { currPosition.x, currPosition.z };
        glm::vec2 posB = posA + (mPedestrian->mPhysicsBody->GetSignVector() * weaponParams.mBaseHitRange);
        // find candidates
        PhysicsLinecastResult linecastResult;
        gPhysics.QueryObjectsLinecast(posA, posB, linecastResult);
        for (int icurr = 0; icurr < linecastResult.mHitsCount; ++icurr)
        {
            PedPhysicsBody* pedBody = linecastResult.mHits[icurr].mPedComponent;
            if (pedBody == nullptr || pedBody->mReferencePed == mPedestrian) // ignore self
                continue; 

            // todo: check distance in y direction
            DamageInfo damageInfo;
            damageInfo.SetDamageFromWeapon(weaponParams, mPedestrian);         
            pedBody->mReferencePed->ReceiveDamage(damageInfo);
        }
    }
    else if (weaponParams.IsRange())
    {
        glm::vec2 signVector = mPedestrian->mPhysicsBody->GetSignVector();       
        glm::vec2 offset = (signVector * 1.0f); //todo: magic numbers
        glm::vec3 projectilePos {
            currPosition.x + offset.x, 
            currPosition.y, 
            currPosition.z + offset.y
        };
        debug_assert(weaponParams.mProjectileTypeID < eProjectileType_COUNT);
        gGameObjectsManager.CreateProjectile(projectilePos, mPedestrian->mPhysicsBody->GetRotationAngle(), &weaponParams);
    }
    else
    {
        debug_assert(false);
    }

    // setup cooldown time for weapons
    float rechargeTime = (1.0f / weaponParams.mBaseFireRate);
    mPedestrian->mWeaponRechargeTime = (currGameTime + rechargeTime);
    return true;
}

ePedestrianState PedestrianStatesManager::GetNextIdleState()
{
    const PedestrianCtlState& ctlState = mPedestrian->mCtlState;
    if (ctlState.mCtlActions[ePedestrianAction_Run])
    {
        if (ctlState.mCtlActions[ePedestrianAction_Shoot])
            return ePedestrianState_RunsAndShoots;

        return ePedestrianState_Runs;
    }

    if (ctlState.mCtlActions[ePedestrianAction_WalkBackward] ||
        ctlState.mCtlActions[ePedestrianAction_WalkForward])
    {
        // cannot walk and use fists simultaneously
        if (ctlState.mCtlActions[ePedestrianAction_Shoot] && mPedestrian->mCurrentWeapon != eWeapon_Fists)
            return ePedestrianState_WalksAndShoots;

        return ePedestrianState_Walks;
    }

    if (ctlState.mCtlActions[ePedestrianAction_Shoot])
        return ePedestrianState_StandsAndShoots;

    return ePedestrianState_StandingStill;
}

ePedestrianAnimID PedestrianStatesManager::DetectIdleAnimation() const
{
    if (mCurrentStateID == ePedestrianState_StandingStill)
        return ePedestrianAnim_StandingStill;

    if (mCurrentStateID == ePedestrianState_Walks)
        return ePedestrianAnim_Walk;

    if (mCurrentStateID == ePedestrianState_Runs)
        return ePedestrianAnim_Run;

    // stands and shoots
    if (mCurrentStateID == ePedestrianState_StandsAndShoots)
    {
        switch (mPedestrian->mCurrentWeapon)
        {
            case eWeapon_Fists: return ePedestrianAnim_PunchingWhileStanding;
            case eWeapon_Pistol: return ePedestrianAnim_ShootPistolWhileStanding;
            case eWeapon_Machinegun: return ePedestrianAnim_ShootMachinegunWhileStanding;
            case eWeapon_Flamethrower: return ePedestrianAnim_ShootFlamethrowerWhileStanding;
            case eWeapon_RocketLauncher: return ePedestrianAnim_ShootRPGWhileStanding;
        }
        debug_assert(false);
        return ePedestrianAnim_StandingStill;
    }

    // walks and shoots
    if (mCurrentStateID == ePedestrianState_WalksAndShoots)
    {
        switch (mPedestrian->mCurrentWeapon)
        {
            case eWeapon_Fists: return ePedestrianAnim_PunchingWhileRunning;
            case eWeapon_Pistol: return ePedestrianAnim_ShootPistolWhileWalking;
            case eWeapon_Machinegun: return ePedestrianAnim_ShootMachinegunWhileWalking;
            case eWeapon_Flamethrower: return ePedestrianAnim_ShootFlamethrowerWhileWalking;
            case eWeapon_RocketLauncher: return ePedestrianAnim_ShootRPGWhileWalking;
        }
        debug_assert(false);
        return ePedestrianAnim_Walk;
    }

    // runs and shoots
    if (mCurrentStateID == ePedestrianState_RunsAndShoots)
    {
        switch (mPedestrian->mCurrentWeapon)
        {
            case eWeapon_Fists: return ePedestrianAnim_PunchingWhileRunning;
            case eWeapon_Pistol: return ePedestrianAnim_ShootPistolWhileRunning;
            case eWeapon_Machinegun: return ePedestrianAnim_ShootMachinegunWhileRunning;
            case eWeapon_Flamethrower: return ePedestrianAnim_ShootFlamethrowerWhileRunning;
            case eWeapon_RocketLauncher: return ePedestrianAnim_ShootRPGWhileRunning;
        }
        debug_assert(false);
        return ePedestrianAnim_Run;
    }

    debug_assert(false);
    return ePedestrianAnim_StandingStill;
}

bool PedestrianStatesManager::CanStartSlideOnCarState() const
{
    return mPedestrian->mPhysicsBody->mContactingCars > 0;
}

void PedestrianStatesManager::SetInCarPositionToDoor()
{
    int doorIndex = mPedestrian->mCurrentCar->GetDoorIndexForSeat(mPedestrian->mCurrentSeat);
    mPedestrian->mCurrentCar->GetDoorPosLocal(doorIndex, mPedestrian->mPhysicsBody->mCarPointLocal);
}

void PedestrianStatesManager::SetInCarPositionToSeat()
{
    mPedestrian->mCurrentCar->GetSeatPosLocal(mPedestrian->mCurrentSeat, mPedestrian->mPhysicsBody->mCarPointLocal);
}

bool PedestrianStatesManager::TryProcessDamage(const DamageInfo& damageInfo)
{
    PedestrianStateEvent eventData(ePedestrianStateEvent_ReceiveDamage);
    eventData.mDamageInfo = damageInfo;

    // handle punch
    if (damageInfo.mDamageCause == eDamageCause_Punch)
    {
        ChangeState(ePedestrianState_KnockedDown, eventData);
        return true;
    }

    // handle fall from height
    if (damageInfo.mDamageCause == eDamageCause_Gravity)
    {
        // todo: check height
        mPedestrian->DieFromDamage(damageInfo.mDamageCause);
        return true;
    }

    // handle high voltage
    if (damageInfo.mDamageCause == eDamageCause_Electricity)
    {
        mPedestrian->DieFromDamage(damageInfo.mDamageCause);
        return true;
    }

    // handle fireball
    if (damageInfo.mDamageCause == eDamageCause_Burning)
    {
        if (mPedestrian->IsBurn()) // already burning
            return false;
        mPedestrian->SetBurnEffectActive(true);
        return true;
    }

    // handle water contact
    if (damageInfo.mDamageCause == eDamageCause_Drowning)
    {
        mPedestrian->DieFromDamage(damageInfo.mDamageCause);
        return true;
    }

    // handle boom
    if (damageInfo.mDamageCause == eDamageCause_Explosion)
    {
        mPedestrian->DieFromDamage(damageInfo.mDamageCause);
        return true;
    }

    // handle bullet
    if (damageInfo.mDamageCause == eDamageCause_Bullet)
    {
        // todo: hitpoints/armor
        mPedestrian->DieFromDamage(damageInfo.mDamageCause);
        return true;
    }

    // handle car hit
    if (damageInfo.mDamageCause == eDamageCause_CarCrash)
    {
        if (damageInfo.mSourceObject == nullptr || !damageInfo.mSourceObject->IsVehicleClass())
            return false;

        Vehicle* vehicle = (Vehicle*)damageInfo.mSourceObject;

        glm::vec2 carPosition = vehicle->mPhysicsBody->GetPosition2();
        glm::vec2 pedPosition = mPedestrian->mPhysicsBody->GetPosition2();
        glm::vec2 directionNormal = glm::normalize(pedPosition - carPosition);
        glm::vec2 directionVelocity = glm::dot(directionNormal, vehicle->mPhysicsBody->GetLinearVelocity()) * directionNormal;
        float speedInDirection = glm::dot(directionVelocity, directionNormal);
        speedInDirection = fabs(speedInDirection);

        float killSpeed = 6.0f; // todo: magic numbers
        if (speedInDirection > killSpeed)
        {
            mPedestrian->DieFromDamage(eDamageCause_CarCrash);
        }
        else if (speedInDirection > 1.0f)// todo: magic numbers
        {
            // jump over
            if (CanStartSlideOnCarState())
            {
                ChangeState(ePedestrianState_SlideOnCar, eventData);
            } 
        }
        return true;
    }

    debug_assert(false);
    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateDead_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    debug_assert(stateEvent.mID == ePedestrianStateEvent_Die);
    mPedestrian->SetAnimation(ePedestrianAnim_LiesOnFloor, eSpriteAnimLoop_FromStart);
    mPedestrian->SetDead(stateEvent.mDeathReason);
    mPedestrian->mPhysicsBody->ClearForces();

    // create effects
    bool createBlood = (stateEvent.mDeathReason != ePedestrianDeathReason_Drowned) &&
        (stateEvent.mDeathReason != ePedestrianDeathReason_Electrocuted) &&
        (stateEvent.mDeathReason != ePedestrianDeathReason_null);

    if (!mPedestrian->IsCarPassenger())
    {
        glm::vec3 position = mPedestrian->mPhysicsBody->GetPosition();
        Decoration* decoration = gGameObjectsManager.CreateFirstBlood(position);
        if (decoration)
        {
            decoration->SetDrawOrder(eSpriteDrawOrder_GroundDecals);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateDriveCar_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    bool isBike = (mPedestrian->mCurrentCar->mCarStyle->mClassID == eVehicleClass_Motorcycle);
    mPedestrian->SetAnimation(isBike ? ePedestrianAnim_SittingOnBike : ePedestrianAnim_SittingInCar, eSpriteAnimLoop_None);

    // dont draw pedestrian if it in car with hard top
    if (mPedestrian->mCurrentCar->HasHardTop())
    {
        eGameObjectFlags flags = mPedestrian->mFlags | eGameObjectFlags_Invisible;
        mPedestrian->mFlags = flags;
    }
}

void PedestrianStatesManager::StateDriveCar_ProcessExit()
{
    // show ped
    if (mPedestrian->mCurrentCar->HasHardTop())
    {
        eGameObjectFlags flags = mPedestrian->mFlags ^ eGameObjectFlags_Invisible;
        mPedestrian->mFlags = flags;
    }
}

bool PedestrianStatesManager::StateDriveCar_ProcessEvent(const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_PullOutFromCar)
    {
        ChangeState(ePedestrianState_KnockedDown, stateEvent);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateExitCar_ProcessFrame()
{
    int doorIndex = mPedestrian->mCurrentCar->GetDoorIndexForSeat(mPedestrian->mCurrentSeat);
    if (mPedestrian->mCurrentCar->HasDoorAnimation(doorIndex) &&
        mPedestrian->mCurrentCar->IsDoorOpened(doorIndex))
    {
        mPedestrian->mCurrentCar->CloseDoor(doorIndex);
    }

    if (!mPedestrian->mCurrentAnimState.IsAnimationActive())
    {
        PedestrianStateEvent evData { ePedestrianStateEvent_None };
        ChangeState(ePedestrianState_StandingStill, evData);
        return;
    }
}

void PedestrianStatesManager::StateExitCar_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    bool isBike = (mPedestrian->mCurrentCar->mCarStyle->mClassID == eVehicleClass_Motorcycle);
    mPedestrian->SetAnimation(isBike ? ePedestrianAnim_ExitBike : ePedestrianAnim_ExitCar, eSpriteAnimLoop_None);

    int doorIndex = mPedestrian->mCurrentCar->GetDoorIndexForSeat(mPedestrian->mCurrentSeat);
    if (mPedestrian->mCurrentCar->HasDoorAnimation(doorIndex))
    {
        mPedestrian->mCurrentCar->OpenDoor(doorIndex);
    }

    SetInCarPositionToDoor();
}

void PedestrianStatesManager::StateExitCar_ProcessExit()
{
    if (mPedestrian->mCurrentCar)
    {
        cxx::angle_t currentCarAngle = mPedestrian->mCurrentCar->mPhysicsBody->GetRotationAngle();

        mPedestrian->mPhysicsBody->SetRotationAngle(currentCarAngle - cxx::angle_t::from_degrees(30.0f));
        mPedestrian->SetCarExited();
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateEnterCar_ProcessFrame()
{
    int doorIndex = mPedestrian->mCurrentCar->GetDoorIndexForSeat(mPedestrian->mCurrentSeat);
    if (mPedestrian->mCurrentCar->HasDoorAnimation(doorIndex) && 
        mPedestrian->mCurrentCar->IsDoorOpened(doorIndex))
    {
        mPedestrian->mCurrentCar->CloseDoor(doorIndex);
    }

    if (mPedestrian->mCurrentAnimState.IsLastFrame())
    {
        SetInCarPositionToSeat();
    }

    if (!mPedestrian->mCurrentAnimState.IsAnimationActive())
    {
        PedestrianStateEvent evData { ePedestrianStateEvent_None };
        ChangeState(ePedestrianState_DrivingCar, evData);
        return;
    }
}

void PedestrianStatesManager::StateEnterCar_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    debug_assert(stateEvent.mTargetCar);

    mPedestrian->SetCarEntered(stateEvent.mTargetCar, stateEvent.mTargetSeat);
    mPedestrian->mPhysicsBody->ClearForces();

    bool isBike = (mPedestrian->mCurrentCar->mCarStyle->mClassID == eVehicleClass_Motorcycle);
    mPedestrian->SetAnimation(isBike ? ePedestrianAnim_EnterBike : ePedestrianAnim_EnterCar, eSpriteAnimLoop_None);

    int doorIndex = mPedestrian->mCurrentCar->GetDoorIndexForSeat(mPedestrian->mCurrentSeat);
    SetInCarPositionToDoor();

    if (mPedestrian->mCurrentCar->HasDoorAnimation(doorIndex))
    {
        mPedestrian->mCurrentCar->OpenDoor(doorIndex);
    }

    // pullout passenger
    if (Pedestrian* prevDriver = mPedestrian->mCurrentCar->GetFirstPassenger(mPedestrian->mCurrentSeat))
    {
        PedestrianStateEvent ev {ePedestrianStateEvent_PullOutFromCar};
        prevDriver->mStatesManager.ProcessEvent(ev);
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateSlideCar_ProcessFrame()
{
    ProcessRotateActions();
    ProcessMotionActions();

    if (mPedestrian->mCurrentAnimID == ePedestrianAnim_JumpOntoCar)
    {
        if (!mPedestrian->mCurrentAnimState.IsAnimationActive())
        {
            mPedestrian->SetAnimation(ePedestrianAnim_SlideOnCar, eSpriteAnimLoop_FromStart);
        }
    }
    else if (mPedestrian->mCurrentAnimID == ePedestrianAnim_SlideOnCar)
    {
        if (!CanStartSlideOnCarState()) // check if no car to slide over
        {
            mPedestrian->SetAnimation(ePedestrianAnim_DropOffCarSliding, eSpriteAnimLoop_None); // end slide
        }
    }
    else if (mPedestrian->mCurrentAnimID == ePedestrianAnim_DropOffCarSliding)
    {
        // check can finish current state
        if (!mPedestrian->mCurrentAnimState.IsAnimationActive())
        {
            PedestrianStateEvent evData { ePedestrianStateEvent_None };
            ChangeState(ePedestrianState_StandingStill, evData);
            return;
        }
    }
    else {}
}

void PedestrianStatesManager::StateSlideCar_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    mPedestrian->SetAnimation(ePedestrianAnim_JumpOntoCar, eSpriteAnimLoop_None);
}

bool PedestrianStatesManager::StateSlideCar_ProcessEvent(const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_FallFromHeightStart)
    {
        ChangeState(ePedestrianState_Falling, stateEvent);
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_WaterContact)
    {
        ChangeState(ePedestrianState_Drowning, stateEvent);
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_ReceiveDamage)
    {
        return TryProcessDamage(stateEvent.mDamageInfo);
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateKnockedDown_ProcessFrame()
{
    if (!mPedestrian->mCurrentAnimState.IsAnimationActive())
    {
        if (mPedestrian->mCurrentAnimID == ePedestrianAnim_FallShort)
        {
            mPedestrian->mPhysicsBody->ClearForces();
            mPedestrian->SetAnimation(ePedestrianAnim_LiesOnFloor, eSpriteAnimLoop_FromStart);
            return;
        }
    }

    if (mPedestrian->mCurrentAnimID == ePedestrianAnim_LiesOnFloor)
    {
        if (mPedestrian->mCurrentStateTime >= gGameParams.mPedestrianKnockedDownTime)
        {
            PedestrianStateEvent evData { ePedestrianStateEvent_None };
            ChangeState(ePedestrianState_StandingStill, evData);
        }
        return;
    }
}

void PedestrianStatesManager::StateKnockedDown_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_PullOutFromCar)
    {
        SetInCarPositionToDoor();

        mPedestrian->SetCarExited();
    }

    float impulse = 0.5f; // todo: magic numbers

    mPedestrian->SetAnimation(ePedestrianAnim_FallShort, eSpriteAnimLoop_None);
    mPedestrian->mPhysicsBody->AddLinearImpulse(-mPedestrian->mPhysicsBody->GetSignVector() * impulse);
}

bool PedestrianStatesManager::StateKnockedDown_ProcessEvent(const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_FallFromHeightStart)
    {
        ChangeState(ePedestrianState_Falling, stateEvent);
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_WaterContact)
    {
        ChangeState(ePedestrianState_Drowning, stateEvent);
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_ReceiveDamage)
    {
        return TryProcessDamage(stateEvent.mDamageInfo);
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateFalling_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    mPedestrian->SetAnimation(ePedestrianAnim_FallLong, eSpriteAnimLoop_FromStart);
}

void PedestrianStatesManager::StateFalling_ProcessExit()
{
    mPedestrian->mPhysicsBody->SetLinearVelocity({}); // force stop
}

bool PedestrianStatesManager::StateFalling_ProcessEvent(const PedestrianStateEvent& stateEvent)
{    
    if (stateEvent.mID == ePedestrianStateEvent_FallFromHeightEnd)
    {
        // die
        if (mPedestrian->mPhysicsBody->mFallDistance > gGameParams.mPedestrianFallDeathHeight - 0.001f)
        {
            mPedestrian->DieFromDamage(eDamageCause_Gravity);
            return true;
        }

        ChangeState(ePedestrianState_StandingStill, stateEvent);
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_WaterContact)
    {
        ChangeState(ePedestrianState_Drowning, stateEvent);
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateIdle_ProcessFrame()
{
    const PedestrianCtlState& ctlState = mPedestrian->mCtlState;

    if (mPedestrian->IsShooting())
    {
        // pedestrian is in shooting state but does he really want to shoot?
        if (ctlState.mCtlActions[ePedestrianAction_Shoot])
        {
            TryToShoot();
        } 
    }

    ProcessRotateActions();
    ProcessMotionActions();

    // slide over car
    if (ctlState.mCtlActions[ePedestrianAction_Run] || ctlState.mCtlActions[ePedestrianAction_WalkForward])
    {
        if (ctlState.mCtlActions[ePedestrianAction_Jump] && CanStartSlideOnCarState())
        {
            PedestrianStateEvent evData { ePedestrianStateEvent_None };
            ChangeState(ePedestrianState_SlideOnCar, evData);
            return;
        }
    }
    
    // process shooting
    ePedestrianState nextIdleState = GetNextIdleState();
    if (nextIdleState == mCurrentStateID)
        return;

    if ((mCurrentStateID == ePedestrianState_RunsAndShoots && nextIdleState == ePedestrianState_Runs) ||
        (mCurrentStateID == ePedestrianState_WalksAndShoots && nextIdleState == ePedestrianState_Walks) ||
        (mCurrentStateID == ePedestrianState_StandsAndShoots && nextIdleState == ePedestrianState_StandingStill))
    {
        // wait animation ends
        if (mPedestrian->mCurrentAnimState.IsAnimationActive() && !mPedestrian->mCurrentAnimState.IsLastFrame())
            return;
    }

    PedestrianStateEvent evData { ePedestrianStateEvent_None };
    ChangeState(nextIdleState, evData);
}

void PedestrianStatesManager::StateIdle_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    ePedestrianAnimID animID = DetectIdleAnimation();
    mPedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

bool PedestrianStatesManager::StateIdle_ProcessEvent(const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_WeaponChange)
    {
        ePedestrianAnimID animID = DetectIdleAnimation();
        mPedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_ReceiveDamage)
    {
        return TryProcessDamage(stateEvent.mDamageInfo);
    }

    if (stateEvent.mID == ePedestrianStateEvent_FallFromHeightStart)
    {
        ChangeState(ePedestrianState_Falling, stateEvent);
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_WaterContact)
    {
        ChangeState(ePedestrianState_Drowning, stateEvent);
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateDrowning_ProcessFrame()
{
    if (gGameParams.mPedestrianDrowningTime < mPedestrian->mCurrentStateTime)
    {
        // force current position to underwater
        glm::vec3 currentPosition = mPedestrian->mPhysicsBody->GetPosition();
        mPedestrian->mPhysicsBody->SetPosition(currentPosition - glm::vec3{0.0f, 2.0f, 0.0f});

        mPedestrian->DieFromDamage(eDamageCause_Drowning);
        return;
    }
}

void PedestrianStatesManager::StateDrowning_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    mPedestrian->SetAnimation(ePedestrianAnim_Drowning, eSpriteAnimLoop_FromStart);
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateDies_ProcessFrame()
{
}

void PedestrianStatesManager::StateDies_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
}

void PedestrianStatesManager::StateDies_ProcessExit()
{
}

bool PedestrianStatesManager::StateDies_ProcessEvent(const PedestrianStateEvent& stateEvent)
{
    return false;
}