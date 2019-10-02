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

        // todo
        case ePedestrianState_DrivingCar:
        case ePedestrianState_SlideOnCar:
        case ePedestrianState_Dying:
        case ePedestrianState_Dead:
            break;
    }
    debug_assert(false);
    return &mIdleState;
}

//////////////////////////////////////////////////////////////////////////

void PedestrianBaseState::ProcessRotateActions(Pedestrian* pedestrian, Timespan deltaTime)
{
    if (pedestrian->mCtlActions[ePedestrianAction_TurnLeft] || 
        pedestrian->mCtlActions[ePedestrianAction_TurnRight])
    {
        float angularVelocity = gGameRules.mPedestrianTurnSpeed;
        if (pedestrian->mCurrentStateID == ePedestrianState_SlideOnCar)
        {
            angularVelocity = gGameRules.mPedestrianTurnSpeedSlideOnCar;
        }
        angularVelocity *= pedestrian->mCtlActions[ePedestrianAction_TurnLeft] ? -1.0f : 1.0f;
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
    switch (pedestrian->mCurrentStateID)
    {
        case ePedestrianState_StandingStill: 
        {
            if (pedestrian->mCtlActions[ePedestrianAction_Run])
            {
                currState = ePedestrianState_Runs;
                break;
            }
            if (pedestrian->mCtlActions[ePedestrianAction_WalkBackward] ||
                pedestrian->mCtlActions[ePedestrianAction_WalkForward])
            {
                currState = ePedestrianState_Walks;
                break;
            }
        }
        break;

        case ePedestrianState_Walks:
        case ePedestrianState_Runs:
        {
            ProcessMotionActions(pedestrian, deltaTime);
            if (pedestrian->mCtlActions[ePedestrianAction_Run])
            {
                currState = ePedestrianState_Runs;
                break;
            }
            if (pedestrian->mCtlActions[ePedestrianAction_WalkBackward] ||
                pedestrian->mCtlActions[ePedestrianAction_WalkForward])
            {
                currState = ePedestrianState_Walks;
                break;
            }
            currState = ePedestrianState_StandingStill;
        }
        break;
    }

    return currState;
}

void PedestrianStateIdle::ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState)
{
    debug_assert(pedestrian->mCurrentStateID == ePedestrianState_StandingStill ||
        pedestrian->mCurrentStateID == ePedestrianState_Walks ||
        pedestrian->mCurrentStateID == ePedestrianState_Runs);

    switch (pedestrian->mCurrentStateID)
    {
        case ePedestrianState_StandingStill: 
            // anim loop
            pedestrian->SetAnimation(eSpriteAnimationID_Ped_StandingStill, eSpriteAnimLoop_FromStart); 
        break;

        case ePedestrianState_Walks:
            // anim loop
            pedestrian->SetAnimation(eSpriteAnimationID_Ped_Walk, eSpriteAnimLoop_FromStart); 
        break;

        case ePedestrianState_Runs:
            // anim loop
            pedestrian->SetAnimation(eSpriteAnimationID_Ped_Run, eSpriteAnimLoop_FromStart); 
        break;
    }
}

void PedestrianStateIdle::ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState)
{
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

    return currState;
}

void PedestrianStateIdleShoots::ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState)
{
    debug_assert(pedestrian->mCurrentStateID == ePedestrianState_StandsAndShoots ||
        pedestrian->mCurrentStateID == ePedestrianState_WalksAndShoots ||
        pedestrian->mCurrentStateID == ePedestrianState_RunsAndShoots);
}

void PedestrianStateIdleShoots::ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState)
{
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
