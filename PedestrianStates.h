#pragma once

#include "GameDefs.h"

// defines basic pedestrian state
class PedestrianBaseState: public cxx::noncopyable
{
public:
    virtual ~PedestrianBaseState()
    {
    }
    // @returns next state
    virtual ePedestrianState ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime);

    virtual void ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState);
    virtual void ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState);
    virtual void ProcessStateWeaponChange(Pedestrian* pedestrian, eWeaponType prevWeapon);

protected:
    void ProcessRotateActions(Pedestrian* pedestrian, Timespan deltaTime);
    void ProcessMotionActions(Pedestrian* pedestrian, Timespan deltaTime);

    // anims helpers
    eSpriteAnimationID DetectStandingStillAnimWithWeapon(eWeaponType weapon, bool shoots) const;
    eSpriteAnimationID DetectWalkingAnimWithWeapon(eWeaponType weapon, bool shoots) const;
    eSpriteAnimationID DetectRunningAnimWithWeapon(eWeaponType weapon, bool shoots) const;
};

// process states:
//    - ePedestrianState_StandingStill
//    - ePedestrianState_Walks
//    - ePedestrianState_Runs

class PedestrianStateIdle: public PedestrianBaseState
{
public:
    ePedestrianState ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;

    void ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState) override;
    void ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState) override;
    void ProcessStateWeaponChange(Pedestrian* pedestrian, eWeaponType prevWeapon) override;
};

// process states:
//    - ePedestrianState_StandsAndShoots
//    - ePedestrianState_WalksAndShoots
//    - ePedestrianState_RunsAndShoots

class PedestrianStateIdleShoots: public PedestrianBaseState
{
public:
    ePedestrianState ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;

    void ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState) override;
    void ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState) override;
    void ProcessStateWeaponChange(Pedestrian* pedestrian, eWeaponType prevWeapon) override;
};

// process states:
//    - ePedestrianState_Falling

class PedestrianStateFalling: public PedestrianBaseState
{
public:
    ePedestrianState ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;

    void ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState) override;
    void ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState) override;
};

// process states:
//    - ePedestrianState_EnteringCar
//    - ePedestrianState_ExitingCar

class PedestrianStateEnterOrExitCar: public PedestrianBaseState
{
public:
    ePedestrianState ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;

    void ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState) override;
    void ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState) override;
};

// process states:
//  - ePedestrianState_SlideOnCar

class PedestrianStateSlideOnCar: public PedestrianBaseState
{
public:
    ePedestrianState ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;

    void ProcessStateEnter(Pedestrian* pedestrian, ePedestrianState previousState) override;
    void ProcessStateExit(Pedestrian* pedestrian, ePedestrianState nextState) override; 
};

class PedestrianBaseStatesManager
{
public:
    PedestrianBaseState* GetStateByID(ePedestrianState stateID);

private:
    PedestrianStateIdle mIdleState;
    PedestrianStateIdleShoots mIdleShootsState;
    PedestrianStateFalling mFallingState;
    PedestrianStateEnterOrExitCar mEnterOrExitCarState;
    PedestrianStateSlideOnCar mSlideOnCarState;
};

extern PedestrianBaseStatesManager gPedestrianBaseStatesManager;

// todo:
    // ePedestrianState_KnockedDown
    //ePedestrianState_DrivingCar,
    //ePedestrianState_Dying,
    //ePedestrianState_Dead,
