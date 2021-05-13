#include "stdafx.h"
#include "PedestrianStates.h"
#include "Pedestrian.h"
#include "PhysicsBody.h"
#include "Vehicle.h"
#include "CarnageGame.h"
#include "PhysicsManager.h"
#include "TimeManager.h"
#include "BroadcastEventsManager.h"
#include "AudioManager.h"
#include "GameObjectHelpers.h"

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
    
    ePedestrianState prevState = mCurrentStateID;
    mPedestrian->mCurrentStateTime = 0;
    debug_assert(nextState > ePedestrianState_Unspecified && nextState < ePedestrianState_COUNT);
    // process exit current state
    (this->*mFuncsTable[mCurrentStateID].pfStateExit)();
    mCurrentStateID = nextState;
    // process enter next state
    (this->*mFuncsTable[mCurrentStateID].pfStateEnter)(evData);
    // notify controller
    if (mPedestrian->mController)
    {
        mPedestrian->mController->OnCharacterChangeState(prevState, mCurrentStateID);
    }
}

bool PedestrianStatesManager::ProcessEvent(const PedestrianStateEvent& evData)
{
    return (this->*mFuncsTable[mCurrentStateID].pfStateEvent)(evData);
}

void PedestrianStatesManager::ProcessFrame()
{
    (this->*mFuncsTable[mCurrentStateID].pfStateFrame)();
}

void PedestrianStatesManager::ProcessSimulationFrame()
{
    (this->*mFuncsTable[mCurrentStateID].pfStateSimulationFrame)();
}

void PedestrianStatesManager::InitFuncsTable()
{
    mFuncsTable[ePedestrianState_Unspecified] = {&PedestrianStatesManager::StateDummy_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateDummy_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_StandingStill] = {&PedestrianStatesManager::StateIdle_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateIdle_ProcessFrame, 
        &PedestrianStatesManager::StateIdle_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_Walks] = {&PedestrianStatesManager::StateIdle_ProcessEnter,
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateIdle_ProcessFrame, 
        &PedestrianStatesManager::StateIdle_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_Runs] = {&PedestrianStatesManager::StateIdle_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit,
        &PedestrianStatesManager::StateIdle_ProcessFrame, 
        &PedestrianStatesManager::StateIdle_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_Falling] = {&PedestrianStatesManager::StateFalling_ProcessEnter, 
        &PedestrianStatesManager::StateFalling_ProcessExit, 
        &PedestrianStatesManager::StateDummy_ProcessFrame, 
        &PedestrianStatesManager::StateFalling_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_EnteringCar] = {&PedestrianStatesManager::StateEnterCar_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateEnterCar_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_ExitingCar] =  {&PedestrianStatesManager::StateExitCar_ProcessEnter, 
        &PedestrianStatesManager::StateExitCar_ProcessExit, 
        &PedestrianStatesManager::StateExitCar_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_DrivingCar] = {&PedestrianStatesManager::StateDriveCar_ProcessEnter, 
        &PedestrianStatesManager::StateDriveCar_ProcessExit, 
        &PedestrianStatesManager::StateDummy_ProcessFrame, 
        &PedestrianStatesManager::StateDriveCar_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_SlideOnCar] = {&PedestrianStatesManager::StateSlideCar_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateSlideCar_ProcessFrame, 
        &PedestrianStatesManager::StateSlideCar_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_Dead] = {&PedestrianStatesManager::StateDead_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateDummy_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_Stunned] = {&PedestrianStatesManager::StateStunned_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateStunned_ProcessFrame, 
        &PedestrianStatesManager::StateStunned_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_Drowning] = {&PedestrianStatesManager::StateDrowning_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateDrowning_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};

    mFuncsTable[ePedestrianState_Electrocuted] = {&PedestrianStatesManager::StateElectrocuted_ProcessEnter, 
        &PedestrianStatesManager::StateDummy_ProcessExit, 
        &PedestrianStatesManager::StateElectrocuted_ProcessFrame, 
        &PedestrianStatesManager::StateDummy_ProcessEvent,
        &PedestrianStatesManager::StateDummy_ProcessSimulationFrame};
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::ProcessRotateActions()
{
    const PedestrianCtlState& ctlState = mPedestrian->GetCtlState();
    if (ctlState.mTurnLeft || ctlState.mTurnRight)
    {
        float turnSpeed = gGameParams.mPedestrianTurnSpeed;
        if (mCurrentStateID == ePedestrianState_SlideOnCar)
        {
            turnSpeed = gGameParams.mPedestrianTurnSpeedSlideOnCar;
        }

        cxx::angle_t angularVelocity = cxx::angle_t::from_degrees(turnSpeed * (ctlState.mTurnLeft ? -1.0f : 1.0f));
        mPedestrian->mPhysicsBody->SetAngularVelocity(angularVelocity);
    }
    else
    {
        cxx::angle_t angularVelocity;
        mPedestrian->mPhysicsBody->SetAngularVelocity(angularVelocity);
    }
}

ePedestrianState PedestrianStatesManager::GetNextIdleState()
{
    const PedestrianCtlState& ctlState = mPedestrian->GetCtlState();
    if (ctlState.mRun)
        return mPedestrian->CanRun() ? ePedestrianState_Runs : ePedestrianState_Walks;

    if (ctlState.mWalkBackward || ctlState.mWalkForward)
        return ePedestrianState_Walks;

    return ePedestrianState_StandingStill;
}

ePedestrianAnimID PedestrianStatesManager::DetectIdleAnimation(bool isShooting) const
{    
    if (mCurrentStateID == ePedestrianState_StandingStill)
    {
        if (isShooting)
        {
            switch (mPedestrian->mCurrentWeapon)
            {
                case eWeapon_Fists: return ePedestrianAnim_PunchingWhileStanding;
                case eWeapon_Pistol: return ePedestrianAnim_ShootPistolWhileStanding;
                case eWeapon_Machinegun: return ePedestrianAnim_ShootMachinegunWhileStanding;
                case eWeapon_Flamethrower: return ePedestrianAnim_ShootFlamethrowerWhileStanding;
                case eWeapon_RocketLauncher: return ePedestrianAnim_ShootRPGWhileStanding;
                default: break;
            }  
        }
        return ePedestrianAnim_StandingStill;
    }

    if (mCurrentStateID == ePedestrianState_Walks)
    {
        if (isShooting)
        {
            switch (mPedestrian->mCurrentWeapon)
            {
                case eWeapon_Fists: return ePedestrianAnim_PunchingWhileRunning;
                case eWeapon_Pistol: return ePedestrianAnim_ShootPistolWhileWalking;
                case eWeapon_Machinegun: return ePedestrianAnim_ShootMachinegunWhileWalking;
                case eWeapon_Flamethrower: return ePedestrianAnim_ShootFlamethrowerWhileWalking;
                case eWeapon_RocketLauncher: return ePedestrianAnim_ShootRPGWhileWalking;
                default: break;
            }
        }
        return ePedestrianAnim_Walk;
    }

    if (mCurrentStateID == ePedestrianState_Runs)
    {
        if (isShooting)
        {
            switch (mPedestrian->mCurrentWeapon)
            {
                case eWeapon_Fists: return ePedestrianAnim_PunchingWhileRunning;
                case eWeapon_Pistol: return ePedestrianAnim_ShootPistolWhileRunning;
                case eWeapon_Machinegun: return ePedestrianAnim_ShootMachinegunWhileRunning;
                case eWeapon_Flamethrower: return ePedestrianAnim_ShootFlamethrowerWhileRunning;
                case eWeapon_RocketLauncher: return ePedestrianAnim_ShootRPGWhileRunning;
                default: break;
            }
        }
        return ePedestrianAnim_Run;
    }

    debug_assert(false);
    return ePedestrianAnim_StandingStill;
}

bool PedestrianStatesManager::CanStartSlideOnCarState() const
{
    return mPedestrian->mContactingCars;
}

void PedestrianStatesManager::SetInCarPositionToDoor()
{
    int doorIndex = mPedestrian->mCurrentCar->GetDoorIndexForSeat(mPedestrian->mCurrentSeat);
    glm::vec2 pos2;
    mPedestrian->mCurrentCar->GetDoorPosLocal(doorIndex, pos2);
    mPedestrian->SetPosition2(pos2);
}

void PedestrianStatesManager::SetInCarPositionToSeat()
{
    glm::vec2 pos2;
    mPedestrian->mCurrentCar->GetSeatPosLocal(mPedestrian->mCurrentSeat, pos2);
    mPedestrian->SetPosition2(pos2);
}

bool PedestrianStatesManager::TryProcessDamage(const DamageInfo& damageInfo)
{
    PedestrianStateEvent eventData(ePedestrianStateEvent_ReceiveDamage);
    eventData.mDamageInfo = damageInfo;

    // handle punch
    if (damageInfo.mDamageCause == eDamageCause_Punch)
    {
        ChangeState(ePedestrianState_Stunned, eventData);
        return true;
    }

    // handle fall from height
    if (damageInfo.mDamageCause == eDamageCause_Gravity)
    {
        if (damageInfo.mFallHeight >= gGameParams.mPedestrianFallDeathHeight)
        {
            mPedestrian->DieFromDamage(damageInfo.mDamageCause);
            return true;
        }
        return false;
    }

    // handle high voltage
    if (damageInfo.mDamageCause == eDamageCause_Electricity)
    {
        ChangeState(ePedestrianState_Electrocuted, eventData);
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
    if (damageInfo.mDamageCause == eDamageCause_CarHit)
    {
        if (!mPedestrian->IsOnTheGround() || mPedestrian->IsDead())
            return false;

        Vehicle* carObject = ToVehicle(damageInfo.mSourceObject);
        debug_assert(carObject);

        glm::vec2 carPosition = carObject->mTransform.GetPosition2();
        glm::vec2 pedPosition = mPedestrian->mTransform.GetPosition2();
        glm::vec2 directionNormal = glm::normalize(pedPosition - carPosition);
        glm::vec2 directionVelocity = glm::dot(directionNormal, carObject->mPhysicsBody->GetLinearVelocity()) * directionNormal;

        float speedInDirection = fabs(glm::dot(directionVelocity, directionNormal));
        float killSpeed = 8.0f; // todo: magic numbers
        if (speedInDirection > killSpeed)
        {
            mPedestrian->StartGameObjectSound(ePedSfxChannelIndex_Voice, eSfxSampleType_Level, SfxLevel_Squashed, SfxFlags_RandomPitch);
            mPedestrian->DieFromDamage(eDamageCause_CarHit);
        }
        else if (speedInDirection > 0.0f)
        {
            // jump over
            if (CanStartSlideOnCarState())
            {
                ChangeState(ePedestrianState_SlideOnCar, eventData);
            } 
        }

        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateDead_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    debug_assert(stateEvent.mID == ePedestrianStateEvent_Die);

    mPedestrian->SetDead(stateEvent.mDeathReason);
    mPedestrian->mPhysicsBody->ClearForces();

    ePedestrianAnimID animID = ePedestrianAnim_LiesOnFloor;
    if (mPedestrian->mDeathReason == ePedestrianDeathReason_Electrocuted)
    {
        animID = ePedestrianAnim_LiesOnFloorBones;
    }
    mPedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart);

    // create effects
    bool createBlood = (stateEvent.mDeathReason != ePedestrianDeathReason_Drowned) &&
        (stateEvent.mDeathReason != ePedestrianDeathReason_Electrocuted) &&
        (stateEvent.mDeathReason != ePedestrianDeathReason_null) && !mPedestrian->IsCarPassenger();

    if (createBlood)
    {
        glm::vec3 position = mPedestrian->mTransform.mPosition;
        Decoration* decoration = gGameObjectsManager.CreateFirstBlood(position);
        if (decoration)
        {
            decoration->SetDrawOrder(eSpriteDrawOrder_GroundDecals);
        }
    }

    if (mPedestrian->IsHumanPlayerCharacter())
    {
        mPedestrian->StartGameObjectSound(ePedSfxChannelIndex_Voice, eSfxSampleType_Voice, SfxVoice_PlayerDies, SfxFlags_None);
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateDriveCar_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    bool isBike = (mPedestrian->mCurrentCar->mCarInfo->mClassID == eVehicleClass_Motorcycle);
    mPedestrian->SetAnimation(isBike ? ePedestrianAnim_SittingOnBike : ePedestrianAnim_SittingInCar, eSpriteAnimLoop_None);

    // dont draw pedestrian if it in car with hard top
    if (mPedestrian->mCurrentCar->HasHardTop())
    {
        GameObjectFlags flags = mPedestrian->mObjectFlags | GameObjectFlags_Invisible;
        mPedestrian->mObjectFlags = flags;
    }
    SetInCarPositionToSeat();
}

void PedestrianStatesManager::StateDriveCar_ProcessExit()
{
    // show ped
    if (mPedestrian->mCurrentCar->HasHardTop())
    {
        GameObjectFlags flags = mPedestrian->mObjectFlags ^ GameObjectFlags_Invisible;
        mPedestrian->mObjectFlags = flags;
    }
}

bool PedestrianStatesManager::StateDriveCar_ProcessEvent(const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_PullOutFromCar)
    {
        ChangeState(ePedestrianState_Stunned, stateEvent);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateExitCar_ProcessFrame()
{
    Vehicle* currentCar = mPedestrian->mCurrentCar;
    int doorIndex = currentCar->GetDoorIndexForSeat(mPedestrian->mCurrentSeat);
    if (currentCar->HasDoorAnimation(doorIndex) &&
        currentCar->IsDoorOpened(doorIndex))
    {
        currentCar->CloseDoor(doorIndex);
    }

    if (!mPedestrian->mCurrentAnimState.IsActive())
    {
        PedestrianStateEvent evData { ePedestrianStateEvent_None };
        ChangeState(ePedestrianState_StandingStill, evData);
        return;
    }
}

void PedestrianStatesManager::StateExitCar_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    bool isBike = (mPedestrian->mCurrentCar->mCarInfo->mClassID == eVehicleClass_Motorcycle);
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
        cxx::angle_t currentCarAngle = mPedestrian->mCurrentCar->mTransform.mOrientation;

        mPedestrian->SetOrientation(currentCarAngle - cxx::angle_t::from_degrees(30.0f));
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

    if (!mPedestrian->mCurrentAnimState.IsActive())
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
    bool isBike = (mPedestrian->mCurrentCar->mCarInfo->mClassID == eVehicleClass_Motorcycle);
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

    if (mPedestrian->mCurrentAnimID == ePedestrianAnim_JumpOntoCar)
    {
        if (!mPedestrian->mCurrentAnimState.IsActive())
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
        if (!mPedestrian->mCurrentAnimState.IsActive())
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

void PedestrianStatesManager::StateStunned_ProcessFrame()
{
    if (!mPedestrian->mCurrentAnimState.IsActive())
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

void PedestrianStatesManager::StateStunned_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_PullOutFromCar)
    {
        SetInCarPositionToDoor();

        mPedestrian->SetCarExited();
    }
    if (stateEvent.mID == ePedestrianStateEvent_ReceiveDamage)
    {
        if (stateEvent.mDamageInfo.mDamageCause == eDamageCause_Punch)
        {
            mPedestrian->StartGameObjectSound(ePedSfxChannelIndex_Voice, eSfxSampleType_Level, SfxLevel_Punch, SfxFlags_RandomPitch);
        }
    }

    float impulse = 0.5f; // todo: magic numbers

    mPedestrian->SetAnimation(ePedestrianAnim_FallShort, eSpriteAnimLoop_None);
    mPedestrian->mPhysicsBody->SetLinearVelocity(-mPedestrian->mTransform.GetDirectionVector() * impulse);
}

bool PedestrianStatesManager::StateStunned_ProcessEvent(const PedestrianStateEvent& stateEvent)
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
    const PedestrianCtlState& ctlState = mPedestrian->GetCtlState();

    bool isShooting = false;
    if (ctlState.mShoot && !mPedestrian->GetWeapon().IsOutOfAmmunition())
    {
        // cannot walk and use fists simultaneously
        isShooting = !((mCurrentStateID == ePedestrianState_Walks) && (mPedestrian->mCurrentWeapon == eWeapon_Fists));
    }

    // do special sounds :)
    if (ctlState.mSpecial)
    {
        SfxSampleIndex specialSound = gCarnageGame.mGameRand.random_chance(50) ? SfxLevel_SpecialSound1 : SfxLevel_SpecialSound2;
        mPedestrian->StartGameObjectSound(ePedSfxChannelIndex_Voice, eSfxSampleType_Level, specialSound, SfxFlags_RandomPitch);
    }

    // update animation
    ePedestrianAnimID animID = DetectIdleAnimation(isShooting);
    if (animID != mPedestrian->mCurrentAnimID)
    {
        // force shoot animation
        if (isShooting)
        {
            mPedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart);
        }
        else if (mPedestrian->mCurrentAnimState.IsLastFrame())
        {
            mPedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
    }

    ProcessRotateActions();

    if (isShooting)
    {
        mPedestrian->GetWeapon().Fire(mPedestrian);
    }

    // slide over car
    if (ctlState.mRun || ctlState.mWalkForward)
    {
        if (ctlState.mJump && CanStartSlideOnCarState())
        {
            PedestrianStateEvent evData { ePedestrianStateEvent_None };
            ChangeState(ePedestrianState_SlideOnCar, evData);
            return;
        }
    }
    
    // check next state
    ePedestrianState nextIdleState = GetNextIdleState();
    if (nextIdleState != mCurrentStateID)
    {
        PedestrianStateEvent evData { ePedestrianStateEvent_None };
        ChangeState(nextIdleState, evData);
    }
}

void PedestrianStatesManager::StateIdle_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    const PedestrianCtlState& ctlState = mPedestrian->GetCtlState();
    bool isShooting = ctlState.mShoot && !mPedestrian->GetWeapon().IsOutOfAmmunition();

    ePedestrianAnimID animID = DetectIdleAnimation(isShooting);
    mPedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

bool PedestrianStatesManager::StateIdle_ProcessEvent(const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_WeaponChange)
    {
        const PedestrianCtlState& ctlState = mPedestrian->GetCtlState();
        bool isShooting = ctlState.mShoot && !mPedestrian->GetWeapon().IsOutOfAmmunition();

        ePedestrianAnimID animID = DetectIdleAnimation(isShooting);
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
        glm::vec3 currentPosition = mPedestrian->mTransform.mPosition;
        mPedestrian->SetPosition(currentPosition - glm::vec3{0.0f, 2.0f, 0.0f});

        mPedestrian->DieFromDamage(eDamageCause_Drowning);
        return;
    }
}

void PedestrianStatesManager::StateDrowning_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    mPedestrian->SetAnimation(ePedestrianAnim_Drowning, eSpriteAnimLoop_FromStart);
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStatesManager::StateElectrocuted_ProcessFrame()
{
    if (!mPedestrian->mCurrentAnimState.IsActive())
    {
        if (mPedestrian->mCurrentAnimID == ePedestrianAnim_FallShort)
        {
            mPedestrian->mPhysicsBody->ClearForces();
            mPedestrian->SetAnimation(ePedestrianAnim_Electrocuted, eSpriteAnimLoop_None);
            return;
        }
        
        if (mPedestrian->mCurrentAnimID == ePedestrianAnim_Electrocuted)
        {
            mPedestrian->DieFromDamage(eDamageCause_Electricity);
            return;
        }
    }
}

void PedestrianStatesManager::StateElectrocuted_ProcessEnter(const PedestrianStateEvent& stateEvent)
{
    float impulse = 0.3f; // todo: magic numbers

    mPedestrian->SetAnimation(ePedestrianAnim_FallShort, eSpriteAnimLoop_None);
    mPedestrian->mPhysicsBody->ClearForces();
    mPedestrian->mPhysicsBody->SetLinearVelocity(-mPedestrian->mTransform.GetDirectionVector() * impulse);
    mPedestrian->StartGameObjectSound(ePedSfxChannelIndex_Voice, eSfxSampleType_Level, SfxLevel_DieScream4, SfxFlags_RandomPitch);
}