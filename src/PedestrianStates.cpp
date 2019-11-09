#include "stdafx.h"
#include "PedestrianStates.h"
#include "Pedestrian.h"
#include "PhysicsComponents.h"
#include "Vehicle.h"
#include "CarnageGame.h"
#include "PhysicsManager.h"

//////////////////////////////////////////////////////////////////////////

PedestrianStateEvent PedestrianStateEvent::Get_ActionWeaponChange(eWeaponType newWeapon)
{
    PedestrianStateEvent eventData {ePedestrianStateEvent_ActionWeaponChange};
    eventData.mActionWeaponChange.mWeapon = newWeapon;

    return eventData;
}

PedestrianStateEvent PedestrianStateEvent::Get_ActionEnterCar(Vehicle* targetCar, eCarSeat targetSeat)
{
    PedestrianStateEvent eventData {ePedestrianStateEvent_ActionEnterCar};
    eventData.mActionEnterCar.mTargetCar = targetCar;
    eventData.mActionEnterCar.mTargetSeat = targetSeat;
    
    return eventData;
}

PedestrianStateEvent PedestrianStateEvent::Get_ActionLeaveCar()
{
    PedestrianStateEvent eventData {ePedestrianStateEvent_ActionLeaveCar};

    return eventData;
}

PedestrianStateEvent PedestrianStateEvent::Get_DamageFromWeapon(eWeaponType weaponType, Pedestrian* attacker)
{
    PedestrianStateEvent eventData {ePedestrianStateEvent_TakeDamageFromWeapon};
    eventData.mDamageFromWeapon.mAttacker = attacker;
    eventData.mDamageFromWeapon.mWeaponType = weaponType;

    return eventData;
}

PedestrianStateEvent PedestrianStateEvent::Get_PullOutFromCar(Pedestrian* attacker)
{
    PedestrianStateEvent eventData {ePedestrianStateEvent_PullOutFromCar};
    eventData.mPullOutFromCar.mAttacker = attacker;

    return eventData;
}

PedestrianStateEvent PedestrianStateEvent::Get_Die(ePedestrianDeathReason reason, Pedestrian* attacker)
{
    PedestrianStateEvent eventData {ePedestrianStateEvent_Die};
    eventData.mDie.mAttacker = attacker;
    eventData.mDie.mDeathReason = reason;

    return eventData;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianBaseState::ProcessRotateActions(Pedestrian* pedestrian, Timespan deltaTime)
{
    if (pedestrian->mCtlActions[ePedestrianAction_TurnLeft] || 
        pedestrian->mCtlActions[ePedestrianAction_TurnRight])
    {
        float angularVelocity = gGameParams.mPedestrianTurnSpeed * (pedestrian->mCtlActions[ePedestrianAction_TurnLeft] ? -1.0f : 1.0f);
        pedestrian->mPhysicsComponent->SetAngularVelocity(angularVelocity);
    }
    else
    {
        pedestrian->mPhysicsComponent->SetAngularVelocity(0.0f);
    }
}

void PedestrianBaseState::ProcessMotionActions(Pedestrian* pedestrian, Timespan deltaTime)
{
    if (pedestrian->mCtlActions[ePedestrianAction_WalkForward] || 
        pedestrian->mCtlActions[ePedestrianAction_WalkBackward] || 
        pedestrian->mCtlActions[ePedestrianAction_Run])
    {
        float moveSpeed = gGameParams.mPedestrianWalkSpeed;

        glm::vec2 linearVelocity = pedestrian->mPhysicsComponent->GetSignVector();
        if (pedestrian->mCtlActions[ePedestrianAction_Run])
        {
            moveSpeed = gGameParams.mPedestrianRunSpeed;
        }
        else if (pedestrian->mCtlActions[ePedestrianAction_WalkBackward])
        {
            linearVelocity = -linearVelocity;
        }
        pedestrian->mPhysicsComponent->SetLinearVelocity(linearVelocity * moveSpeed);
    }
    else
    {
        pedestrian->mPhysicsComponent->SetLinearVelocity({}); // force stop
    }
}

eSpriteAnimID PedestrianBaseState::DetectStandingStillAnimWithWeapon(eWeaponType weapon, bool shoots) const
{
    if (!shoots)
        return eSpriteAnimID_Ped_StandingStill;

    switch (weapon)
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

eSpriteAnimID PedestrianBaseState::DetectWalkingAnimWithWeapon(eWeaponType weapon, bool shoots) const
{
    if (!shoots)
        return eSpriteAnimID_Ped_Walk;

    switch (weapon)
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

eSpriteAnimID PedestrianBaseState::DetectRunningAnimWithWeapon(eWeaponType weapon, bool shoots) const
{
    if (!shoots)
        return eSpriteAnimID_Ped_Run;

    switch (weapon)
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

bool PedestrianBaseState::CanStartSlideOnCarState(Pedestrian* pedestrian) const
{
    return pedestrian->mPhysicsComponent->mContactingCars > 0;
}

void PedestrianBaseState::SetInCarPositionToDoor(Pedestrian* pedestrian)
{
    int doorIndex = pedestrian->mCurrentCar->GetDoorIndexForSeat(pedestrian->mCurrentSeat);

    pedestrian->mCurrentCar->GetDoorPosLocal(doorIndex, pedestrian->mPhysicsComponent->mCarPointLocal);
}

void PedestrianBaseState::SetInCarPositionToSeat(Pedestrian* pedestrian)
{
    pedestrian->mCurrentCar->GetSeatPosLocal(pedestrian->mCurrentSeat, pedestrian->mPhysicsComponent->mCarPointLocal);
}

//////////////////////////////////////////////////////////////////////////

PedestrianBaseState* PedestrianStateIdleBase::GetNextIdleState(Pedestrian* pedestrian, Timespan deltaTime)
{
    if (pedestrian->mCtlActions[ePedestrianAction_Run])
    {
        if (pedestrian->mCtlActions[ePedestrianAction_Shoot])
            return &pedestrian->mStateRunsAndShoots;

        return &pedestrian->mStateRuns;
    }

    if (pedestrian->mCtlActions[ePedestrianAction_WalkBackward] ||
        pedestrian->mCtlActions[ePedestrianAction_WalkForward])
    {
        // cannot walk and use fists simultaneously
        if (pedestrian->mCtlActions[ePedestrianAction_Shoot] && pedestrian->mCurrentWeapon != eWeaponType_Fists)
            return &pedestrian->mStateWalksAndShoots;

        return &pedestrian->mStateWalks;
    }

    if (pedestrian->mCtlActions[ePedestrianAction_Shoot])
        return &pedestrian->mStateStandsAndShoots;

    return &pedestrian->mStateStandingStill;
}

void PedestrianStateIdleBase::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    // check for falling state
    if (pedestrian->mPhysicsComponent->mFalling)
    {
        pedestrian->ChangeState(&pedestrian->mStateFalling, nullptr);
        return;
    }

    if (pedestrian->IsShooting())
    {
        TryToShoot(pedestrian);
    }

    ProcessRotateActions(pedestrian, deltaTime);
    ProcessMotionActions(pedestrian, deltaTime);

    // slide over car
    if (pedestrian->mCtlActions[ePedestrianAction_Run] || pedestrian->mCtlActions[ePedestrianAction_WalkForward])
    {
        if (pedestrian->mCtlActions[ePedestrianAction_Jump] && CanStartSlideOnCarState(pedestrian))
        {
            pedestrian->ChangeState(&pedestrian->mStateSlideOnCar, nullptr);
            return;
        }
    }
    
    // process shooting
    PedestrianBaseState* nextIdleState = GetNextIdleState(pedestrian, deltaTime);
    if (nextIdleState == pedestrian->mCurrentState)
        return;

    ePedestrianState currPedestrianState = pedestrian->mCurrentState->GetStateID();

    if ((currPedestrianState == ePedestrianState_RunsAndShoots && nextIdleState->GetStateID() == ePedestrianState_Runs) ||
        (currPedestrianState == ePedestrianState_WalksAndShoots && nextIdleState->GetStateID() == ePedestrianState_Walks) ||
        (currPedestrianState == ePedestrianState_StandsAndShoots && nextIdleState->GetStateID() == ePedestrianState_StandingStill))
    {
        // wait animation ends
        if (pedestrian->mCurrentAnimState.IsAnimationActive() && !pedestrian->mCurrentAnimState.IsLastFrame())
            return;
    }

    pedestrian->ChangeState(nextIdleState, nullptr);
}

bool PedestrianStateIdleBase::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_TakeDamageFromWeapon)
    {
        pedestrian->ChangeState(&pedestrian->mStateKnockedDown, &stateEvent);
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_ActionEnterCar)
    {
        pedestrian->ChangeState(&pedestrian->mStateEnterCar, &stateEvent);
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        pedestrian->SetCurrentWeapon(stateEvent.mActionWeaponChange.mWeapon);
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_Die)
    {
        pedestrian->SetDead(stateEvent.mDie.mDeathReason);
        return true;
    }

    return false;
}

bool PedestrianStateIdleBase::TryToShoot(Pedestrian* pedestrian)
{
    Timespan currGameTime = gCarnageGame.mGameTime;
    if (pedestrian->mWeaponRechargeTime > currGameTime ||
        pedestrian->mWeaponsAmmo[pedestrian->mCurrentWeapon] == 0)
    {
        return false;
    }

    if (pedestrian->mCurrentWeapon == eWeaponType_Fists)
    {
        glm::vec3 pos = pedestrian->mPhysicsComponent->GetPosition();
        glm::vec2 posA { pos.x, pos.z };
        glm::vec2 posB = posA + (pedestrian->mPhysicsComponent->GetSignVector() * gGameParams.mWeaponsDistance[pedestrian->mCurrentWeapon]);
        // find candidates
        PhysicsLinecastResult linecastResult;
        gPhysics.QueryObjectsLinecast(posA, posB, linecastResult);
        for (int icurr = 0; icurr < linecastResult.mHitsCount; ++icurr)
        {
            PedPhysicsComponent* pedBody = linecastResult.mHits[icurr].mPedComponent;
            if (pedBody == nullptr || pedBody->mReferencePed == pedestrian) // ignore self
                continue; 

            // todo: check distance in y direction

            PedestrianStateEvent ev = PedestrianStateEvent::Get_DamageFromWeapon(pedestrian->mCurrentWeapon, pedestrian);
            pedBody->mReferencePed->ProcessEvent(ev);
        }
    }
    else
    {
        // todo: add projectiles
    }    

    // setup cooldown time for weapons
    Timespan rechargeTime = Timespan::FromSeconds(gGameParams.mWeaponsRechargeTime[pedestrian->mCurrentWeapon]);
    pedestrian->mWeaponRechargeTime = currGameTime + rechargeTime;
    return true;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateStandingStill::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimID animID = DetectStandingStillAnimWithWeapon(pedestrian->mCurrentWeapon, false);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

bool PedestrianStateStandingStill::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimID animID = DetectStandingStillAnimWithWeapon(stateEvent.mActionWeaponChange.mWeapon, false);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        pedestrian->SetCurrentWeapon(stateEvent.mActionWeaponChange.mWeapon);
        return true;
    }

    return PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateWalks::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimID animID = DetectWalkingAnimWithWeapon(pedestrian->mCurrentWeapon, false);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

bool PedestrianStateWalks::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimID animID = DetectWalkingAnimWithWeapon(stateEvent.mActionWeaponChange.mWeapon, false);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart);
        pedestrian->SetCurrentWeapon(stateEvent.mActionWeaponChange.mWeapon);
        return true;
    }

    return PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateRuns::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimID animID = DetectRunningAnimWithWeapon(pedestrian->mCurrentWeapon, false);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

bool PedestrianStateRuns::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimID animID = DetectRunningAnimWithWeapon(stateEvent.mActionWeaponChange.mWeapon, false);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        pedestrian->SetCurrentWeapon(stateEvent.mActionWeaponChange.mWeapon);
        return true;
    }

    return PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateStandsAndShoots::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimID animID = DetectStandingStillAnimWithWeapon(pedestrian->mCurrentWeapon, true);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

bool PedestrianStateStandsAndShoots::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimID animID = DetectStandingStillAnimWithWeapon(stateEvent.mActionWeaponChange.mWeapon, true);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        pedestrian->SetCurrentWeapon(stateEvent.mActionWeaponChange.mWeapon);
        return true;
    }

    return PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateWalksAndShoots::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimID animID = DetectWalkingAnimWithWeapon(pedestrian->mCurrentWeapon, true);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

bool PedestrianStateWalksAndShoots::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimID animID = DetectWalkingAnimWithWeapon(stateEvent.mActionWeaponChange.mWeapon, true);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        pedestrian->SetCurrentWeapon(stateEvent.mActionWeaponChange.mWeapon);
        return true;
    }

    return PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateRunsAndShoots::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimID animID = DetectRunningAnimWithWeapon(pedestrian->mCurrentWeapon, true);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

bool PedestrianStateRunsAndShoots::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimID animID = DetectRunningAnimWithWeapon(stateEvent.mActionWeaponChange.mWeapon, true);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        pedestrian->SetCurrentWeapon(stateEvent.mActionWeaponChange.mWeapon);
        return true;
    }

    return PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateFalling::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    if (!pedestrian->mPhysicsComponent->mFalling)
    {
        pedestrian->ChangeState(&pedestrian->mStateStandingStill, nullptr);
        return;
    }
}

void PedestrianStateFalling::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    pedestrian->SetAnimation(eSpriteAnimID_Ped_FallLong, eSpriteAnimLoop_FromStart);
}

void PedestrianStateFalling::ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    pedestrian->mPhysicsComponent->SetLinearVelocity({}); // force stop
}

bool PedestrianStateFalling::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_Die)
    {
        pedestrian->SetDead(stateEvent.mDie.mDeathReason);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateKnockedDown::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    // check for falling state
    if (pedestrian->mPhysicsComponent->mFalling)
    {
        pedestrian->ChangeState(&pedestrian->mStateFalling, nullptr);
        return;
    }

    if (!pedestrian->mCurrentAnimState.IsAnimationActive())
    {
        if (pedestrian->mCurrentAnimID == eSpriteAnimID_Ped_FallShort)
        {
            pedestrian->mPhysicsComponent->ClearForces();
            pedestrian->SetAnimation(eSpriteAnimID_Ped_LiesOnFloor, eSpriteAnimLoop_FromStart);
            return;
        }
    }

    if (pedestrian->mCurrentAnimID == eSpriteAnimID_Ped_LiesOnFloor)
    {
        if (pedestrian->mCurrentStateTime >= Timespan::FromSeconds(gGameParams.mPedestrianKnockedDownTime))
        {
            pedestrian->ChangeState(&pedestrian->mStateStandingStill, nullptr);
        }
        return;
    }
}

void PedestrianStateKnockedDown::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    float impulse = 0.6f;

    pedestrian->SetAnimation(eSpriteAnimID_Ped_FallShort, eSpriteAnimLoop_None);
    pedestrian->mPhysicsComponent->AddLinearImpulse(-pedestrian->mPhysicsComponent->GetSignVector() * impulse);
}

bool PedestrianStateKnockedDown::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_Die)
    {
        pedestrian->SetDead(stateEvent.mDie.mDeathReason);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateSlideOnCar::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    // check for falling state
    if (pedestrian->mPhysicsComponent->mFalling)
    {
        pedestrian->ChangeState(&pedestrian->mStateFalling, nullptr);
        return;
    }
    
    ProcessRotateActions(pedestrian, deltaTime);
    ProcessMotionActions(pedestrian, deltaTime);

    if (pedestrian->mCurrentAnimID == eSpriteAnimID_Ped_JumpOntoCar)
    {
        if (!pedestrian->mCurrentAnimState.IsAnimationActive())
        {
            pedestrian->SetAnimation(eSpriteAnimID_Ped_SlideOnCar, eSpriteAnimLoop_FromStart);
        }
    }
    else if (pedestrian->mCurrentAnimID == eSpriteAnimID_Ped_SlideOnCar)
    {
        if (!CanStartSlideOnCarState(pedestrian)) // check if no car to slide over
        {
            pedestrian->SetAnimation(eSpriteAnimID_Ped_DropOffCarSliding, eSpriteAnimLoop_None); // end slide
        }
    }
    else if (pedestrian->mCurrentAnimID == eSpriteAnimID_Ped_DropOffCarSliding)
    {
        // check can finish current state
        if (!pedestrian->mCurrentAnimState.IsAnimationActive())
        {
            pedestrian->ChangeState(&pedestrian->mStateStandingStill, nullptr);
            return;
        }
    }
    else {}
}

void PedestrianStateSlideOnCar::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    pedestrian->SetAnimation(eSpriteAnimID_Ped_JumpOntoCar, eSpriteAnimLoop_None);
}

bool PedestrianStateSlideOnCar::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_Die)
    {
        pedestrian->SetDead(stateEvent.mDie.mDeathReason);
        return true;
    }
    return false;
}

void PedestrianStateSlideOnCar::ProcessRotateActions(Pedestrian* pedestrian, Timespan deltaTime)
{
    if (pedestrian->mCtlActions[ePedestrianAction_TurnLeft] || 
        pedestrian->mCtlActions[ePedestrianAction_TurnRight])
    {
        float angularVelocity = gGameParams.mPedestrianTurnSpeedSlideOnCar * (pedestrian->mCtlActions[ePedestrianAction_TurnLeft] ? -1.0f : 1.0f);
        pedestrian->mPhysicsComponent->SetAngularVelocity(angularVelocity);
    }
    else
    {
        pedestrian->mPhysicsComponent->SetAngularVelocity(0.0f);
    }
}

void PedestrianStateSlideOnCar::ProcessMotionActions(Pedestrian* pedestrian, Timespan deltaTime)
{
    glm::vec2 linearVelocity = gGameParams.mPedestrianSlideOnCarSpeed * pedestrian->mPhysicsComponent->GetSignVector();
    pedestrian->mPhysicsComponent->SetLinearVelocity(linearVelocity);
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateEnterCar::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    int doorIndex = pedestrian->mCurrentCar->GetDoorIndexForSeat(pedestrian->mCurrentSeat);
    if (pedestrian->mCurrentCar->HasDoorAnimation(doorIndex) && 
        pedestrian->mCurrentCar->IsDoorOpened(doorIndex))
    {
        pedestrian->mCurrentCar->CloseDoor(doorIndex);
    }

    if (pedestrian->mCurrentAnimState.IsLastFrame())
    {
        SetInCarPositionToSeat(pedestrian);
    }

    if (!pedestrian->mCurrentAnimState.IsAnimationActive())
    {
        pedestrian->ChangeState(&pedestrian->mStateDrivingCar, nullptr);
        return;
    }
}

void PedestrianStateEnterCar::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    if (transitionEvent == nullptr || transitionEvent->mID != ePedestrianStateEvent_ActionEnterCar)
    {
        debug_assert(false);
        return;
    }

    if (transitionEvent->mActionEnterCar.mTargetCar == nullptr || 
        transitionEvent->mActionEnterCar.mTargetSeat == eCarSeat_Any)
    {
        debug_assert(false);

        pedestrian->ChangeState(&pedestrian->mStateStandingStill, nullptr);
        return;
    }

    pedestrian->SetCarEntered(transitionEvent->mActionEnterCar.mTargetCar, transitionEvent->mActionEnterCar.mTargetSeat);
    pedestrian->mPhysicsComponent->ClearForces();

    bool isBike = (pedestrian->mCurrentCar->mCarStyle->mVType == eCarVType_Motorcycle);
    pedestrian->SetAnimation(isBike ? eSpriteAnimID_Ped_EnterBike : eSpriteAnimID_Ped_EnterCar, eSpriteAnimLoop_None);

    int doorIndex = pedestrian->mCurrentCar->GetDoorIndexForSeat(pedestrian->mCurrentSeat);
    SetInCarPositionToDoor(pedestrian);

    if (pedestrian->mCurrentCar->HasDoorAnimation(doorIndex))
    {
        pedestrian->mCurrentCar->OpenDoor(doorIndex);
    }

    // pullout driver
    if (Pedestrian* prevDriver = pedestrian->mCurrentCar->GetFirstPassenger(pedestrian->mCurrentSeat))
    {
        PedestrianStateEvent ev = PedestrianStateEvent::Get_PullOutFromCar(pedestrian);
        prevDriver->ProcessEvent(ev);
    }
}

bool PedestrianStateEnterCar::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_Die)
    {
        pedestrian->SetDead(stateEvent.mDie.mDeathReason);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateExitCar::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    int doorIndex = pedestrian->mCurrentCar->GetDoorIndexForSeat(pedestrian->mCurrentSeat);
    if (pedestrian->mCurrentCar->HasDoorAnimation(doorIndex) &&
        pedestrian->mCurrentCar->IsDoorOpened(doorIndex))
    {
        pedestrian->mCurrentCar->CloseDoor(doorIndex);
    }

    if (!pedestrian->mCurrentAnimState.IsAnimationActive())
    {
        pedestrian->ChangeState(&pedestrian->mStateStandingStill, nullptr);
        return;
    }
}

void PedestrianStateExitCar::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    bool isBike = (pedestrian->mCurrentCar->mCarStyle->mVType == eCarVType_Motorcycle);
    pedestrian->SetAnimation(isBike ? eSpriteAnimID_Ped_ExitBike : eSpriteAnimID_Ped_ExitCar, eSpriteAnimLoop_None);

    int doorIndex = pedestrian->mCurrentCar->GetDoorIndexForSeat(pedestrian->mCurrentSeat);
    if (pedestrian->mCurrentCar->HasDoorAnimation(doorIndex))
    {
        pedestrian->mCurrentCar->OpenDoor(doorIndex);
    }

    SetInCarPositionToDoor(pedestrian);
}

void PedestrianStateExitCar::ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    if (pedestrian->mCurrentCar)
    {
        cxx::angle_t currentCarAngle = pedestrian->mCurrentCar->mPhysicsComponent->GetRotationAngle();

        pedestrian->mPhysicsComponent->SetRotationAngle(currentCarAngle - cxx::angle_t::from_degrees(30.0f));
        pedestrian->SetCarExited();
    }
}

bool PedestrianStateExitCar::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_Die)
    {
        pedestrian->SetDead(stateEvent.mDie.mDeathReason);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateDrivingCar::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    pedestrian->mCurrentCar->PutPassenger(pedestrian, pedestrian->mCurrentSeat);

    bool isBike = (pedestrian->mCurrentCar->mCarStyle->mVType == eCarVType_Motorcycle);
    pedestrian->SetAnimation(isBike ? eSpriteAnimID_Ped_SittingOnBike : eSpriteAnimID_Ped_SittingInCar, eSpriteAnimLoop_None);
}

void PedestrianStateDrivingCar::ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    pedestrian->mCurrentCar->RemovePassenger(pedestrian);
    if (transitionEvent && transitionEvent->mID == ePedestrianStateEvent_PullOutFromCar)
    {
        SetInCarPositionToDoor(pedestrian);

        pedestrian->SetCarExited();
    }
}

bool PedestrianStateDrivingCar::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionLeaveCar)
    {
        pedestrian->ChangeState(&pedestrian->mStateExitCar, &stateEvent);
        return true;
    }

    if (stateEvent.mID == ePedestrianStateEvent_PullOutFromCar)
    {
        pedestrian->ChangeState(&pedestrian->mStateKnockedDown, &stateEvent);
        return true;
    }
    if (stateEvent.mID == ePedestrianStateEvent_Die)
    {
        pedestrian->SetDead(stateEvent.mDie.mDeathReason);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateDead::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    // todo
}

void PedestrianStateDead::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    // todo
}

void PedestrianStateDead::ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    // todo
}

bool PedestrianStateDead::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    return false; // ignore all
}
