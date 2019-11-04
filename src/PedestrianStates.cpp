#include "stdafx.h"
#include "PedestrianStates.h"
#include "Pedestrian.h"
#include "PhysicsComponents.h"
#include "Vehicle.h"
#include "CarnageGame.h"
#include "PhysicsManager.h"

//////////////////////////////////////////////////////////////////////////

PedestrianStateEvent PedestrianStateEvent::Get_ActionWeaponChange(eWeaponType prevWeapon)
{
    PedestrianStateEvent eventData {ePedestrianStateEvent_ActionWeaponChange};
    eventData.mActionWeaponChange.mPrevWeapon = prevWeapon;

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

eSpriteAnimationID PedestrianBaseState::DetectStandingStillAnimWithWeapon(eWeaponType weapon, bool shoots) const
{
    if (!shoots)
        return eSpriteAnimationID_Ped_StandingStill;

    switch (weapon)
    {
        case eWeaponType_Fists: return eSpriteAnimationID_Ped_PunchingWhileStanding;
        case eWeaponType_Pistol: return eSpriteAnimationID_Ped_ShootPistolWhileStanding;
        case eWeaponType_Machinegun: return eSpriteAnimationID_Ped_ShootMachinegunWhileStanding;
        case eWeaponType_Flamethrower: return eSpriteAnimationID_Ped_ShootFlamethrowerWhileStanding;
        case eWeaponType_RocketLauncher: return eSpriteAnimationID_Ped_ShootRPGWhileStanding;
    }
    debug_assert(false);
    return eSpriteAnimationID_Ped_StandingStill;
}

eSpriteAnimationID PedestrianBaseState::DetectWalkingAnimWithWeapon(eWeaponType weapon, bool shoots) const
{
    if (!shoots)
        return eSpriteAnimationID_Ped_Walk;

    switch (weapon)
    {
        case eWeaponType_Fists: return eSpriteAnimationID_Ped_PunchingWhileRunning;
        case eWeaponType_Pistol: return eSpriteAnimationID_Ped_ShootPistolWhileWalking;
        case eWeaponType_Machinegun: return eSpriteAnimationID_Ped_ShootMachinegunWhileWalking;
        case eWeaponType_Flamethrower: return eSpriteAnimationID_Ped_ShootFlamethrowerWhileWalking;
        case eWeaponType_RocketLauncher: return eSpriteAnimationID_Ped_ShootRPGWhileWalking;
    }
    debug_assert(false);
    return eSpriteAnimationID_Ped_Walk;
}

eSpriteAnimationID PedestrianBaseState::DetectRunningAnimWithWeapon(eWeaponType weapon, bool shoots) const
{
    if (!shoots)
        return eSpriteAnimationID_Ped_Run;

    switch (weapon)
    {
        case eWeaponType_Fists: return eSpriteAnimationID_Ped_PunchingWhileRunning;
        case eWeaponType_Pistol: return eSpriteAnimationID_Ped_ShootPistolWhileRunning;
        case eWeaponType_Machinegun: return eSpriteAnimationID_Ped_ShootMachinegunWhileRunning;
        case eWeaponType_Flamethrower: return eSpriteAnimationID_Ped_ShootFlamethrowerWhileRunning;
        case eWeaponType_RocketLauncher: return eSpriteAnimationID_Ped_ShootRPGWhileRunning;
    }
    debug_assert(false);
    return eSpriteAnimationID_Ped_Run;
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

    if (!CanInterrupCurrentIdleAnim(pedestrian))
        return;

    ProcessMotionActions(pedestrian, deltaTime);

    if (pedestrian->mCtlActions[ePedestrianAction_Run] || pedestrian->mCtlActions[ePedestrianAction_WalkForward])
    {
        if (pedestrian->mCtlActions[ePedestrianAction_Jump] && CanStartSlideOnCarState(pedestrian))
        {
            pedestrian->ChangeState(&pedestrian->mStateSlideOnCar, nullptr);
            return;
        }
    }
 
    if (pedestrian->mCtlActions[ePedestrianAction_Run])
    {
        if (pedestrian->mCtlActions[ePedestrianAction_Shoot])
        {
            pedestrian->ChangeState(&pedestrian->mStateRunsAndShoots, nullptr);
            return;
        }
        pedestrian->ChangeState(&pedestrian->mStateRuns, nullptr);
        return;
    }

    if (pedestrian->mCtlActions[ePedestrianAction_WalkBackward] ||
        pedestrian->mCtlActions[ePedestrianAction_WalkForward])
    {
        // cannot walk and use fists simultaneously
        if (pedestrian->mCtlActions[ePedestrianAction_Shoot] && pedestrian->mCurrentWeapon != eWeaponType_Fists)
        {
            pedestrian->ChangeState(&pedestrian->mStateWalksAndShoots, nullptr);
            return;
        }
        pedestrian->ChangeState(&pedestrian->mStateWalks, nullptr);
        return;
    }

    if (pedestrian->mCtlActions[ePedestrianAction_Shoot])
    {
        pedestrian->ChangeState(&pedestrian->mStateStandsAndShoots, nullptr);
        return;
    }

    pedestrian->ChangeState(&pedestrian->mStateStandingStill, nullptr);
    return;
}

void PedestrianStateIdleBase::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_TakeDamageFromWeapon)
    {
        pedestrian->ChangeState(&pedestrian->mStateKnockedDown, &stateEvent);
        return;
    }

    if (!CanInterrupCurrentIdleAnim(pedestrian))
        return;

    if (stateEvent.mID == ePedestrianStateEvent_ActionEnterCar)
    {
        pedestrian->ChangeState(&pedestrian->mStateEnterCar, &stateEvent);
    }
}

bool PedestrianStateIdleBase::CanInterrupCurrentIdleAnim(Pedestrian* pedestrian) const
{
    if (pedestrian->IsStanding() && pedestrian->IsShooting())
    {
        return pedestrian->mCurrentAnimState.IsLastFrame() || 
            !pedestrian->mCurrentAnimState.IsAnimationActive();
    }
    return true;
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
        PhysicsQueryResult queryResult;
        gPhysics.QueryObjects(posA, posB, queryResult);
        for (int iped = 0; iped < queryResult.mPedsCount; ++iped)
        {
            Pedestrian* currPedestrian = queryResult.mPedsList[iped]->mReferencePed;
            if (currPedestrian == pedestrian) // ignore self
                continue;

            // todo: check distance in y direction

            currPedestrian->TakeDamage(pedestrian->mCurrentWeapon, pedestrian);
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
    eSpriteAnimationID animID = DetectStandingStillAnimWithWeapon(pedestrian->mCurrentWeapon, false);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

void PedestrianStateStandingStill::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimationID animID = DetectStandingStillAnimWithWeapon(pedestrian->mCurrentWeapon, false);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
    }
    else
    {
        PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateWalks::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimationID animID = DetectWalkingAnimWithWeapon(pedestrian->mCurrentWeapon, false);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

void PedestrianStateWalks::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimationID animID = DetectWalkingAnimWithWeapon(pedestrian->mCurrentWeapon, false);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
    }
    else
    {
        PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateRuns::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimationID animID = DetectRunningAnimWithWeapon(pedestrian->mCurrentWeapon, false);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

void PedestrianStateRuns::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimationID animID = DetectRunningAnimWithWeapon(pedestrian->mCurrentWeapon, false);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
    }
    else
    {
        PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateStandsAndShoots::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimationID animID = DetectStandingStillAnimWithWeapon(pedestrian->mCurrentWeapon, true);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

void PedestrianStateStandsAndShoots::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimationID animID = DetectStandingStillAnimWithWeapon(pedestrian->mCurrentWeapon, true);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
    }
    else
    {
        PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateWalksAndShoots::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimationID animID = DetectWalkingAnimWithWeapon(pedestrian->mCurrentWeapon, true);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

void PedestrianStateWalksAndShoots::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimationID animID = DetectWalkingAnimWithWeapon(pedestrian->mCurrentWeapon, true);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
    }
    else
    {
        PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateRunsAndShoots::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    eSpriteAnimationID animID = DetectRunningAnimWithWeapon(pedestrian->mCurrentWeapon, true);
    pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
}

void PedestrianStateRunsAndShoots::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionWeaponChange)
    {
        eSpriteAnimationID animID = DetectRunningAnimWithWeapon(pedestrian->mCurrentWeapon, true);
        pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
    }
    else
    {
        PedestrianStateIdleBase::ProcessStateEvent(pedestrian, stateEvent);
    }
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
    pedestrian->SetAnimation(eSpriteAnimationID_Ped_FallLong, eSpriteAnimLoop_FromStart);
}

void PedestrianStateFalling::ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    pedestrian->mPhysicsComponent->SetLinearVelocity({}); // force stop
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateKnockedDown::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    if (!pedestrian->mCurrentAnimState.IsAnimationActive())
    {
        if (pedestrian->mCurrentAnimID == eSpriteAnimationID_Ped_FallShort)
        {
            pedestrian->mPhysicsComponent->ClearForces();
            pedestrian->SetAnimation(eSpriteAnimationID_Ped_LiesOnFloor, eSpriteAnimLoop_FromStart);
            return;
        }
    }

    if (pedestrian->mCurrentAnimID == eSpriteAnimationID_Ped_LiesOnFloor)
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
    pedestrian->SetAnimation(eSpriteAnimationID_Ped_FallShort, eSpriteAnimLoop_None);
    pedestrian->mPhysicsComponent->AddLinearImpulse(-pedestrian->mPhysicsComponent->GetSignVector() * 0.3f);
}

void PedestrianStateKnockedDown::ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
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

    if (pedestrian->mCurrentAnimID == eSpriteAnimationID_Ped_JumpOntoCar)
    {
        if (!pedestrian->mCurrentAnimState.IsAnimationActive())
        {
            pedestrian->SetAnimation(eSpriteAnimationID_Ped_SlideOnCar, eSpriteAnimLoop_FromStart);
        }
    }
    else if (pedestrian->mCurrentAnimID == eSpriteAnimationID_Ped_SlideOnCar)
    {
        if (!CanStartSlideOnCarState(pedestrian)) // check if no car to slide over
        {
            pedestrian->SetAnimation(eSpriteAnimationID_Ped_DropOffCarSliding, eSpriteAnimLoop_None); // end slide
        }
    }
    else if (pedestrian->mCurrentAnimID == eSpriteAnimationID_Ped_DropOffCarSliding)
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
    pedestrian->SetAnimation(eSpriteAnimationID_Ped_JumpOntoCar, eSpriteAnimLoop_None);
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

    pedestrian->mCurrentCar = transitionEvent->mActionEnterCar.mTargetCar;
    pedestrian->mCurrentSeat = transitionEvent->mActionEnterCar.mTargetSeat;
    pedestrian->mPhysicsComponent->ClearForces();

    bool isBike = (pedestrian->mCurrentCar->mCarStyle->mVType == eCarVType_Motorcycle);
    pedestrian->SetAnimation(isBike ? eSpriteAnimationID_Ped_EnterBike : eSpriteAnimationID_Ped_EnterCar, eSpriteAnimLoop_None);

    int doorIndex = pedestrian->mCurrentCar->GetDoorIndexForSeat(pedestrian->mCurrentSeat);
    SetInCarPositionToDoor(pedestrian);

    if (pedestrian->mCurrentCar->HasDoorAnimation(doorIndex))
    {
        pedestrian->mCurrentCar->OpenDoor(doorIndex);
    }

    // pullout driver
    if (Pedestrian* prevDriver = pedestrian->mCurrentCar->GetFirstPassenger(pedestrian->mCurrentSeat))
    {
        prevDriver->PullOutFromCar(pedestrian);
    }
}

void PedestrianStateEnterCar::ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    if (pedestrian->mCurrentCar)
    {
        pedestrian->HandleCarEntered();
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateExitCar::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    int doorIndex = pedestrian->mCurrentCar->GetDoorIndexForSeat(pedestrian->mCurrentSeat);
    if (mIsPullOut)
    {
        // do nothing
    }
    else
    {
        if (pedestrian->mCurrentCar->HasDoorAnimation(doorIndex) &&
            pedestrian->mCurrentCar->IsDoorOpened(doorIndex))
        {
            pedestrian->mCurrentCar->CloseDoor(doorIndex);
        }
    }

    if (!pedestrian->mCurrentAnimState.IsAnimationActive())
    {
        if (mIsPullOut)
        {
            pedestrian->ChangeState(&pedestrian->mStateKnockedDown, nullptr);
        }
        else
        {
            pedestrian->ChangeState(&pedestrian->mStateStandingStill, nullptr);
        }
        return;
    }
}

void PedestrianStateExitCar::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    mIsPullOut = (transitionEvent && transitionEvent->mID == ePedestrianStateEvent_PullOutFromCar);

    int doorIndex = pedestrian->mCurrentCar->GetDoorIndexForSeat(pedestrian->mCurrentSeat);
    if (mIsPullOut)
    {
        pedestrian->SetAnimation(eSpriteAnimationID_Ped_FallShort, eSpriteAnimLoop_None);
    }
    else
    {
        bool isBike = (pedestrian->mCurrentCar->mCarStyle->mVType == eCarVType_Motorcycle);
        pedestrian->SetAnimation(isBike ? eSpriteAnimationID_Ped_ExitBike : eSpriteAnimationID_Ped_ExitCar, eSpriteAnimLoop_None);

        if (pedestrian->mCurrentCar->HasDoorAnimation(doorIndex))
        {
            pedestrian->mCurrentCar->OpenDoor(doorIndex);
        }
    }

    SetInCarPositionToDoor(pedestrian);
}

void PedestrianStateExitCar::ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    if (pedestrian->mCurrentCar)
    {
        pedestrian->HandleCarExited();

        cxx::angle_t currentCarAngle = pedestrian->mCurrentCar->mPhysicsComponent->GetRotationAngle();
        if (mIsPullOut)
        {
            pedestrian->mPhysicsComponent->SetRotationAngle(currentCarAngle + cxx::angle_t::from_degrees(30.0f));
        }
        else
        {
            pedestrian->mPhysicsComponent->SetRotationAngle(currentCarAngle - cxx::angle_t::from_degrees(30.0f));
        }
        pedestrian->mCurrentCar = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////

void PedestrianStateDrivingCar::ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    pedestrian->mCurrentCar->PutPassenger(pedestrian, pedestrian->mCurrentSeat);

    bool isBike = (pedestrian->mCurrentCar->mCarStyle->mVType == eCarVType_Motorcycle);
    pedestrian->SetAnimation(isBike ? eSpriteAnimationID_Ped_SittingOnBike : eSpriteAnimationID_Ped_SittingInCar, eSpriteAnimLoop_None);
}

void PedestrianStateDrivingCar::ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent)
{
    pedestrian->mCurrentCar->RemovePassenger(pedestrian);
}

void PedestrianStateDrivingCar::ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent)
{
    if (stateEvent.mID == ePedestrianStateEvent_ActionLeaveCar)
    {
        pedestrian->ChangeState(&pedestrian->mStateExitCar, &stateEvent);
        return;
    }

    if (stateEvent.mID == ePedestrianStateEvent_PullOutFromCar)
    {
        pedestrian->ChangeState(&pedestrian->mStateExitCar, &stateEvent);
        return;
    }
}
