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
        &PedestrianStatesManager::StateDummy_ProcessExit, 
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
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::ProcessRotateActions()
{
    if (mPedestrian->mCtlActions[ePedestrianAction_TurnLeft] || 
        mPedestrian->mCtlActions[ePedestrianAction_TurnRight])
    {
        float turnSpeed = gGameParams.mPedestrianTurnSpeed;
        if (mCurrentStateID == ePedestrianState_SlideOnCar)
        {
            turnSpeed = gGameParams.mPedestrianTurnSpeedSlideOnCar;
        }

        cxx::angle_t angularVelocity = cxx::angle_t::from_degrees(turnSpeed * (mPedestrian->mCtlActions[ePedestrianAction_TurnLeft] ? -1.0f : 1.0f));
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
    // while slding on car
    if (mCurrentStateID == ePedestrianState_SlideOnCar)
    {
        glm::vec2 linearVelocity = gGameParams.mPedestrianSlideOnCarSpeed * mPedestrian->mPhysicsBody->GetSignVector();
        mPedestrian->mPhysicsBody->SetLinearVelocity(linearVelocity);
        return;
    }

    glm::vec2 linearVelocity {};
    // generic case
    if (mPedestrian->mCtlActions[ePedestrianAction_WalkForward] || 
        mPedestrian->mCtlActions[ePedestrianAction_WalkBackward] || 
        mPedestrian->mCtlActions[ePedestrianAction_Run])
    {
        float moveSpeed = gGameParams.mPedestrianWalkSpeed;

        linearVelocity = mPedestrian->mPhysicsBody->GetSignVector();
        if (mPedestrian->mCtlActions[ePedestrianAction_Run])
        {
            moveSpeed = gGameParams.mPedestrianRunSpeed;
        }
        else if (mPedestrian->mCtlActions[ePedestrianAction_WalkBackward])
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
    debug_assert(mPedestrian->mCurrentWeapon < eWeaponType_COUNT);

    float currGameTime = gTimeManager.mGameTime;
    if (mPedestrian->mWeaponRechargeTime >= currGameTime ||
        mPedestrian->mWeaponsAmmo[mPedestrian->mCurrentWeapon] == 0)
    {
        return false;
    }

    glm::vec3 currPosition = mPedestrian->mPhysicsBody->GetPosition();

    // get weapon params
    WeaponStyle& weaponParams = gGameMap.mStyleData.mWeapons[mPedestrian->mCurrentWeapon];

    if (weaponParams.mFireTypeID == eWeaponFireType_Melee)
    {
        glm::vec2 posA { currPosition.x, currPosition.z };
        glm::vec2 posB = posA + (mPedestrian->mPhysicsBody->GetSignVector() * weaponParams.mBaseMeleeHitDistance);
        // find candidates
        PhysicsLinecastResult linecastResult;
        gPhysics.QueryObjectsLinecast(posA, posB, linecastResult);
        for (int icurr = 0; icurr < linecastResult.mHitsCount; ++icurr)
        {
            PedPhysicsBody* pedBody = linecastResult.mHits[icurr].mPedComponent;
            if (pedBody == nullptr || pedBody->mReferencePed == mPedestrian) // ignore self
                continue; 

            // todo: check distance in y direction

            pedBody->mReferencePed->ReceiveDamage(mPedestrian->mCurrentWeapon, mPedestrian);
        }
    }
    else if (weaponParams.mFireTypeID == eWeaponFireType_Projectile)
    {
        glm::vec2 signVector = mPedestrian->mPhysicsBody->GetSignVector();       
        glm::vec2 offset = (signVector * 1.0f); //todo: magic numbers
        glm::vec3 projectilePos {
            currPosition.x + offset.x, 
            currPosition.y, 
            currPosition.z + offset.y
        };
        debug_assert(weaponParams.mProjectileID < eProjectileType_COUNT);
        gGameObjectsManager.CreateProjectile(projectilePos, mPedestrian->mPhysicsBody->GetRotationAngle(), weaponParams.mProjectileID);
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
    if (mPedestrian->mCtlActions[ePedestrianAction_Run])
    {
        if (mPedestrian->mCtlActions[ePedestrianAction_Shoot])
            return ePedestrianState_RunsAndShoots;

        return ePedestrianState_Runs;
    }

    if (mPedestrian->mCtlActions[ePedestrianAction_WalkBackward] ||
        mPedestrian->mCtlActions[ePedestrianAction_WalkForward])
    {
        // cannot walk and use fists simultaneously
        if (mPedestrian->mCtlActions[ePedestrianAction_Shoot] && mPedestrian->mCurrentWeapon != eWeaponType_Fists)
            return ePedestrianState_WalksAndShoots;

        return ePedestrianState_Walks;
    }

    if (mPedestrian->mCtlActions[ePedestrianAction_Shoot])
        return ePedestrianState_StandsAndShoots;

    return ePedestrianState_StandingStill;
}

eSpriteAnimID PedestrianStatesManager::DetectIdleAnimation() const
{
    if (mCurrentStateID == ePedestrianState_StandingStill)
        return eSpriteAnimID_Ped_StandingStill;

    if (mCurrentStateID == ePedestrianState_Walks)
        return eSpriteAnimID_Ped_Walk;

    if (mCurrentStateID == ePedestrianState_Runs)
        return eSpriteAnimID_Ped_Run;

    // stands and shoots
    if (mCurrentStateID == ePedestrianState_StandsAndShoots)
    {
        switch (mPedestrian->mCurrentWeapon)
        {
            case eWeaponType_Fists: return eSpriteAnimID_Ped_PunchingWhileStanding;
            case eWeaponType_Pistol: return eSpriteAnimID_Ped_ShootPistolWhileStanding;
            case eWeaponType_Machinegun: return eSpriteAnimID_Ped_ShootMachinegunWhileStanding;
            case eWeaponType_Flamethrower: return eSpriteAnimID_Ped_ShootFlamethrowerWhileStanding;
            case eWeaponType_RocketLauncher: return eSpriteAnimID_Ped_ShootRPGWhileStanding;
        }
        debug_assert(false);
        return eSpriteAnimID_Ped_StandingStill;
    }

    // walks and shoots
    if (mCurrentStateID == ePedestrianState_WalksAndShoots)
    {
        switch (mPedestrian->mCurrentWeapon)
        {
            case eWeaponType_Fists: return eSpriteAnimID_Ped_PunchingWhileRunning;
            case eWeaponType_Pistol: return eSpriteAnimID_Ped_ShootPistolWhileWalking;
            case eWeaponType_Machinegun: return eSpriteAnimID_Ped_ShootMachinegunWhileWalking;
            case eWeaponType_Flamethrower: return eSpriteAnimID_Ped_ShootFlamethrowerWhileWalking;
            case eWeaponType_RocketLauncher: return eSpriteAnimID_Ped_ShootRPGWhileWalking;
        }
        debug_assert(false);
        return eSpriteAnimID_Ped_Walk;
    }

    // runs and shoots
    if (mCurrentStateID == ePedestrianState_RunsAndShoots)
    {
        switch (mPedestrian->mCurrentWeapon)
        {
            case eWeaponType_Fists: return eSpriteAnimID_Ped_PunchingWhileRunning;
            case eWeaponType_Pistol: return eSpriteAnimID_Ped_ShootPistolWhileRunning;
            case eWeaponType_Machinegun: return eSpriteAnimID_Ped_ShootMachinegunWhileRunning;
            case eWeaponType_Flamethrower: return eSpriteAnimID_Ped_ShootFlamethrowerWhileRunning;
            case eWeaponType_RocketLauncher: return eSpriteAnimID_Ped_ShootRPGWhileRunning;
        }
        debug_assert(false);
        return eSpriteAnimID_Ped_Run;
    }

    debug_assert(false);
    return eSpriteAnimID_Ped_StandingStill;
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

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateDead_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    debug_assert(stateEvent.mID == ePedestrianStateEvent_Die);
    mPedestrian->SetAnimation(eSpriteAnimID_Ped_LiesOnFloor, eSpriteAnimLoop_FromStart);
    mPedestrian->SetDead(stateEvent.mDeathReason);
    mPedestrian->mPhysicsBody->ClearForces();
}

//////////////////////////////////////////////////////////////////////////


void PedestrianStatesManager::StateDriveCar_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    bool isBike = (mPedestrian->mCurrentCar->mCarStyle->mVType == eCarVType_Motorcycle);
    mPedestrian->SetAnimation(isBike ? eSpriteAnimID_Ped_SittingOnBike : eSpriteAnimID_Ped_SittingInCar, eSpriteAnimLoop_None);
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
    bool isBike = (mPedestrian->mCurrentCar->mCarStyle->mVType == eCarVType_Motorcycle);
    mPedestrian->SetAnimation(isBike ? eSpriteAnimID_Ped_ExitBike : eSpriteAnimID_Ped_ExitCar, eSpriteAnimLoop_None);

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

    bool isBike = (mPedestrian->mCurrentCar->mCarStyle->mVType == eCarVType_Motorcycle);
    mPedestrian->SetAnimation(isBike ? eSpriteAnimID_Ped_EnterBike : eSpriteAnimID_Ped_EnterCar, eSpriteAnimLoop_None);

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

    if (mPedestrian->mCurrentAnimID == eSpriteAnimID_Ped_JumpOntoCar)
    {
        if (!mPedestrian->mCurrentAnimState.IsAnimationActive())
        {
            mPedestrian->SetAnimation(eSpriteAnimID_Ped_SlideOnCar, eSpriteAnimLoop_FromStart);
        }
    }
    else if (mPedestrian->mCurrentAnimID == eSpriteAnimID_Ped_SlideOnCar)
    {
        if (!CanStartSlideOnCarState()) // check if no car to slide over
        {
            mPedestrian->SetAnimation(eSpriteAnimID_Ped_DropOffCarSliding, eSpriteAnimLoop_None); // end slide
        }
    }
    else if (mPedestrian->mCurrentAnimID == eSpriteAnimID_Ped_DropOffCarSliding)
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
    mPedestrian->SetAnimation(eSpriteAnimID_Ped_JumpOntoCar, eSpriteAnimLoop_None);
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

    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateKnockedDown_ProcessFrame()
{
    if (!mPedestrian->mCurrentAnimState.IsAnimationActive())
    {
        if (mPedestrian->mCurrentAnimID == eSpriteAnimID_Ped_FallShort)
        {
            mPedestrian->mPhysicsBody->ClearForces();
            mPedestrian->SetAnimation(eSpriteAnimID_Ped_LiesOnFloor, eSpriteAnimLoop_FromStart);
            return;
        }
    }

    if (mPedestrian->mCurrentAnimID == eSpriteAnimID_Ped_LiesOnFloor)
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

    mPedestrian->SetAnimation(eSpriteAnimID_Ped_FallShort, eSpriteAnimLoop_None);
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

    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateFalling_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    mPedestrian->SetAnimation(eSpriteAnimID_Ped_FallLong, eSpriteAnimLoop_FromStart);
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
            mPedestrian->Die(ePedestrianDeathReason_FallFromHeight, nullptr);
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
    if (mPedestrian->IsShooting())
    {
        TryToShoot();
    }

    ProcessRotateActions();
    ProcessMotionActions();

    // slide over car
    if (mPedestrian->mCtlActions[ePedestrianAction_Run] || mPedestrian->mCtlActions[ePedestrianAction_WalkForward])
    {
        if (mPedestrian->mCtlActions[ePedestrianAction_Jump] && CanStartSlideOnCarState())
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
    eSpriteAnimID animID = DetectIdleAnimation();
    mPedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

bool PedestrianStatesManager::StateIdle_ProcessEvent(const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_WeaponChange)
    {
        eSpriteAnimID animID = DetectIdleAnimation();
        mPedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_DamageFromWeapon)
    {
        ChangeState(ePedestrianState_KnockedDown, stateEvent);
        return true;
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

        mPedestrian->Die(ePedestrianDeathReason_Drowned, nullptr);
        return;
    }
}

void PedestrianStatesManager::StateDrowning_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    mPedestrian->SetAnimation(eSpriteAnimID_Ped_Drowning, eSpriteAnimLoop_FromStart);
}