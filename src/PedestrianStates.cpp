#include "stdafx.h"
#include "PedestrianStates.h"
#include "Pedestrian.h"
#include "PhysicsComponents.h"

PedestrianBaseStatesManager gPedestrianBaseStatesManager;

PedestrianBaseState* PedestrianBaseStatesManager::GetStateByID(ePedestrianState stateID)
{
    switch (stateID)
    {
        case ePedestrianState_StandingStill:
        case ePedestrianState_Walks:
        case ePedestrianState_Runs: return &mIdleState;

        case ePedestrianState_StandsAndShoots:
        case ePedestrianState_WalksAndShoots:
        case ePedestrianState_RunsAndShoots: return &mIdleShootsState;

        case ePedestrianState_Falling: return &mFallingState;

        case ePedestrianState_EnteringCar:
        case ePedestrianState_ExitingCar: return &mEnterOrExitCarState;

        case ePedestrianState_SlideOnCar: return &mSlideOnCarState;
        // todo
        case ePedestrianState_DrivingCar:
        case ePedestrianState_Dying:
        case ePedestrianState_Dead:
            break;
    }
    debug_assert(false);
    return &mIdleState;
}

//////////////////////////////////////////////////////////////////////////

ePedestrianState PedestrianBaseState::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    return pedestrian->mCurrentStateID;
}

void PedestrianBaseState::ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState)
{
    // does nothing
}

void PedestrianBaseState::ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState)
{
    // does nothing
}

void PedestrianBaseState::ProcessStateWeaponChange(Pedestrian* pedestrian, eWeaponType prevWeapon)
{
    // does nothing
}

void PedestrianBaseState::ProcessRotateActions(Pedestrian* pedestrian, Timespan deltaTime)
{
    if (pedestrian->mCtlActions[ePedestrianAction_TurnLeft] || 
        pedestrian->mCtlActions[ePedestrianAction_TurnRight])
    {
        float angularVelocity = gGameRules.mPedestrianTurnSpeed * (pedestrian->mCtlActions[ePedestrianAction_TurnLeft] ? -1.0f : 1.0f);
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
        float moveSpeed = gGameRules.mPedestrianWalkSpeed;

        glm::vec2 linearVelocity = pedestrian->mPhysicsComponent->GetSignVector();
        if (pedestrian->mCtlActions[ePedestrianAction_Run])
        {
            moveSpeed = gGameRules.mPedestrianRunSpeed;
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

//////////////////////////////////////////////////////////////////////////

ePedestrianState PedestrianStateIdle::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    ePedestrianState currState = pedestrian->mCurrentStateID;

    // check for falling state
    if (pedestrian->mPhysicsComponent->mFalling)
    {
        return ePedestrianState_Falling;
    }

    ProcessRotateActions(pedestrian, deltaTime);
    ProcessMotionActions(pedestrian, deltaTime);

    if (pedestrian->mCtlActions[ePedestrianAction_Run] || pedestrian->mCtlActions[ePedestrianAction_WalkForward])
    {
        if (pedestrian->mCtlActions[ePedestrianAction_Jump] && CanStartSlideOnCarState(pedestrian))
            return ePedestrianState_SlideOnCar;
    }
 
    if (pedestrian->mCtlActions[ePedestrianAction_Run])
    {
        if (pedestrian->mCtlActions[ePedestrianAction_Shoot])
            return ePedestrianState_RunsAndShoots;

        return ePedestrianState_Runs;
    }

    if (pedestrian->mCtlActions[ePedestrianAction_WalkBackward] ||
        pedestrian->mCtlActions[ePedestrianAction_WalkForward])
    {
        // cannot walk and use fists simultaneously
        if (pedestrian->mCtlActions[ePedestrianAction_Shoot] && pedestrian->mCurrentWeapon != eWeaponType_Fists)
            return ePedestrianState_WalksAndShoots;

        return ePedestrianState_Walks;
    }

    if (pedestrian->mCtlActions[ePedestrianAction_Shoot])
        return ePedestrianState_StandsAndShoots;

    return ePedestrianState_StandingStill;
}

void PedestrianStateIdle::ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState)
{
    debug_assert(pedestrian->mCurrentStateID == ePedestrianState_StandingStill ||
        pedestrian->mCurrentStateID == ePedestrianState_Walks ||
        pedestrian->mCurrentStateID == ePedestrianState_Runs);

    switch (pedestrian->mCurrentStateID)
    {
        case ePedestrianState_StandingStill: 
        {
            eSpriteAnimationID animID = DetectStandingStillAnimWithWeapon(pedestrian->mCurrentWeapon, false);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;

        case ePedestrianState_Walks:
        {
            eSpriteAnimationID animID = DetectWalkingAnimWithWeapon(pedestrian->mCurrentWeapon, false);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;

        case ePedestrianState_Runs:
        {
            eSpriteAnimationID animID = DetectRunningAnimWithWeapon(pedestrian->mCurrentWeapon, false);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;
    }
}

void PedestrianStateIdle::ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState)
{
}

void PedestrianStateIdle::ProcessStateWeaponChange(Pedestrian* pedestrian, eWeaponType prevWeapon)
{
    switch (pedestrian->mCurrentStateID)
    {
        case ePedestrianState_StandingStill: 
        {
            eSpriteAnimationID animID = DetectStandingStillAnimWithWeapon(pedestrian->mCurrentWeapon, false);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;

        case ePedestrianState_Walks:
        {
            eSpriteAnimationID animID = DetectWalkingAnimWithWeapon(pedestrian->mCurrentWeapon, false);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;

        case ePedestrianState_Runs:
        {
            eSpriteAnimationID animID = DetectRunningAnimWithWeapon(pedestrian->mCurrentWeapon, false);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////

ePedestrianState PedestrianStateIdleShoots::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    ePedestrianState currState = pedestrian->mCurrentStateID;

    // check for falling state
    if (pedestrian->mPhysicsComponent->mFalling)
    {
        return ePedestrianState_Falling;
    }

    ProcessRotateActions(pedestrian, deltaTime);
    if (currState == ePedestrianState_WalksAndShoots || currState == ePedestrianState_RunsAndShoots)
    {
        ProcessMotionActions(pedestrian, deltaTime);
    }

    if (pedestrian->mCtlActions[ePedestrianAction_Run] || pedestrian->mCtlActions[ePedestrianAction_WalkForward])
    {
        if (pedestrian->mCtlActions[ePedestrianAction_Jump] && CanStartSlideOnCarState(pedestrian))
            return ePedestrianState_SlideOnCar;
    }
 
    if (pedestrian->mCtlActions[ePedestrianAction_Run])
    {
        if (pedestrian->mCtlActions[ePedestrianAction_Shoot])
            return ePedestrianState_RunsAndShoots;

        return ePedestrianState_Runs;
    }

    if (pedestrian->mCtlActions[ePedestrianAction_WalkBackward] ||
        pedestrian->mCtlActions[ePedestrianAction_WalkForward])
    {
        // cannot walk and use fists simultaneously
        if (pedestrian->mCtlActions[ePedestrianAction_Shoot] && pedestrian->mCurrentWeapon != eWeaponType_Fists)
            return ePedestrianState_WalksAndShoots;

        return ePedestrianState_Walks;
    }

    if (pedestrian->mCtlActions[ePedestrianAction_Shoot])
        return ePedestrianState_StandsAndShoots;

    return ePedestrianState_StandingStill;
}

void PedestrianStateIdleShoots::ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState)
{
    debug_assert(pedestrian->mCurrentStateID == ePedestrianState_StandsAndShoots ||
        pedestrian->mCurrentStateID == ePedestrianState_WalksAndShoots ||
        pedestrian->mCurrentStateID == ePedestrianState_RunsAndShoots);

    switch (pedestrian->mCurrentStateID)
    {
        case ePedestrianState_StandsAndShoots: 
        {
            eSpriteAnimationID animID = DetectStandingStillAnimWithWeapon(pedestrian->mCurrentWeapon, true);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;

        case ePedestrianState_WalksAndShoots:
        {
            eSpriteAnimationID animID = DetectWalkingAnimWithWeapon(pedestrian->mCurrentWeapon, true);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;

        case ePedestrianState_RunsAndShoots:
        {
            eSpriteAnimationID animID = DetectRunningAnimWithWeapon(pedestrian->mCurrentWeapon, true);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;
    }
}

void PedestrianStateIdleShoots::ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState)
{
}

void PedestrianStateIdleShoots::ProcessStateWeaponChange(Pedestrian* pedestrian, eWeaponType prevWeapon)
{
    switch (pedestrian->mCurrentStateID)
    {
        case ePedestrianState_StandsAndShoots: 
        {
            eSpriteAnimationID animID = DetectStandingStillAnimWithWeapon(pedestrian->mCurrentWeapon, true);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;

        case ePedestrianState_WalksAndShoots:
        {
            eSpriteAnimationID animID = DetectWalkingAnimWithWeapon(pedestrian->mCurrentWeapon, true);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;

        case ePedestrianState_RunsAndShoots:
        {
            eSpriteAnimationID animID = DetectRunningAnimWithWeapon(pedestrian->mCurrentWeapon, true);
            pedestrian->SetAnimation(animID, eSpriteAnimLoop_FromStart); 
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////

ePedestrianState PedestrianStateFalling::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    ePedestrianState currState = pedestrian->mCurrentStateID;
    if (!pedestrian->mPhysicsComponent->mFalling)
    {
        currState = ePedestrianState_StandingStill;
    }
    return currState;
}

void PedestrianStateFalling::ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState)
{
    debug_assert(pedestrian->mCurrentStateID == ePedestrianState_Falling);
    pedestrian->SetAnimation(eSpriteAnimationID_Ped_FallLong, eSpriteAnimLoop_FromStart);
}

void PedestrianStateFalling::ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState)
{
    pedestrian->mPhysicsComponent->SetLinearVelocity({}); // force stop
}

//////////////////////////////////////////////////////////////////////////

ePedestrianState PedestrianStateEnterOrExitCar::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    ePedestrianState currState = pedestrian->mCurrentStateID;

    return currState;
}

void PedestrianStateEnterOrExitCar::ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState)
{
    debug_assert(pedestrian->mCurrentStateID == ePedestrianState_EnteringCar || 
        pedestrian->mCurrentStateID == ePedestrianState_ExitingCar);
}

void PedestrianStateEnterOrExitCar::ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState)
{
}

//////////////////////////////////////////////////////////////////////////

ePedestrianState PedestrianStateSlideOnCar::ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime)
{
    ePedestrianState currState = pedestrian->mCurrentStateID;

    // check for falling state
    if (pedestrian->mPhysicsComponent->mFalling)
    {
        return ePedestrianState_Falling;
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
            currState = ePedestrianState_StandingStill;
        }
    }

    return currState;
}

void PedestrianStateSlideOnCar::ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState)
{
    debug_assert(pedestrian->mCurrentStateID == ePedestrianState_SlideOnCar);
    pedestrian->SetAnimation(eSpriteAnimationID_Ped_JumpOntoCar, eSpriteAnimLoop_None);
}

void PedestrianStateSlideOnCar::ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState)
{
}

void PedestrianStateSlideOnCar::ProcessRotateActions(Pedestrian* pedestrian, Timespan deltaTime)
{
    if (pedestrian->mCtlActions[ePedestrianAction_TurnLeft] || 
        pedestrian->mCtlActions[ePedestrianAction_TurnRight])
    {
        float angularVelocity = gGameRules.mPedestrianTurnSpeedSlideOnCar * (pedestrian->mCtlActions[ePedestrianAction_TurnLeft] ? -1.0f : 1.0f);
        pedestrian->mPhysicsComponent->SetAngularVelocity(angularVelocity);
    }
    else
    {
        pedestrian->mPhysicsComponent->SetAngularVelocity(0.0f);
    }
}

void PedestrianStateSlideOnCar::ProcessMotionActions(Pedestrian* pedestrian, Timespan deltaTime)
{
    glm::vec2 linearVelocity = gGameRules.mPedestrianSlideOnCarSpeed * pedestrian->mPhysicsComponent->GetSignVector();
    pedestrian->mPhysicsComponent->SetLinearVelocity(linearVelocity);
}