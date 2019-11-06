#pragma once

#include "GameDefs.h"

enum ePedestrianStateEvent
{
    ePedestrianStateEvent_ActionWeaponChange, 
    ePedestrianStateEvent_ActionEnterCar, // brains request enter specific vehicle
    ePedestrianStateEvent_ActionLeaveCar, // brains request exit current vehicle

    ePedestrianStateEvent_TakeDamageFromWeapon,
    ePedestrianStateEvent_PullOutFromCar,
};

// defines state event
struct PedestrianStateEvent
{
public:
    // build state event helpers
    static PedestrianStateEvent Get_ActionWeaponChange(eWeaponType newWeapon);
    static PedestrianStateEvent Get_ActionEnterCar(Vehicle* targetCar, eCarSeat targetSeat);
    static PedestrianStateEvent Get_ActionLeaveCar();
    static PedestrianStateEvent Get_DamageFromWeapon(eWeaponType weaponType, Pedestrian* attacker);
    static PedestrianStateEvent Get_PullOutFromCar(Pedestrian* attacker);

    PedestrianStateEvent(ePedestrianStateEvent eventID): mID(eventID)
    {
    }
public:
    ePedestrianStateEvent mID;

    // data for event ePedestrianStateEvent_ActionWeaponChange
    struct _action_weapon_change
    {
        eWeaponType mWeapon;
    };
    _action_weapon_change mActionWeaponChange;

    // data for event ePedestrianStateEvent_ActionEnterCar
    struct _action_enter_car
    {
        Vehicle* mTargetCar = nullptr;
        eCarSeat mTargetSeat;
    };
    _action_enter_car mActionEnterCar;

    // data for event ePedestrianStateEvent_TakeDamageFromWeapon
    struct _damage_from_weapon
    {
        Pedestrian* mAttacker = nullptr;
        eWeaponType mWeaponType;
    };
    _damage_from_weapon mDamageFromWeapon;

    // data for event ePedestrianStateEvent_PullOutFromCar
    struct _pullout_from_car
    {
        Pedestrian* mAttacker = nullptr;
    };
    _pullout_from_car mPullOutFromCar;
};

// defines basic pedestrian state
class PedestrianBaseState: public cxx::noncopyable
{
public:
    virtual ~PedestrianBaseState()
    {
    }
    // get actual state identifier
    inline ePedestrianState GetStateID() const { return mStateIdentifier; }

    // @param transitionEvent: optional, but some states will require for startup params
    virtual void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) { }
    virtual void ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) { }
    virtual void ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) { }
    // send event to state
    // @returns false if event is ignored by state
    virtual bool ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent);

protected:
    PedestrianBaseState(ePedestrianState stateIdentifier) : mStateIdentifier(stateIdentifier) {}
    virtual void ProcessRotateActions(Pedestrian* pedestrian, Timespan deltaTime);
    virtual void ProcessMotionActions(Pedestrian* pedestrian, Timespan deltaTime);

    // anims helpers
    eSpriteAnimationID DetectStandingStillAnimWithWeapon(eWeaponType weapon, bool shoots) const;
    eSpriteAnimationID DetectWalkingAnimWithWeapon(eWeaponType weapon, bool shoots) const;
    eSpriteAnimationID DetectRunningAnimWithWeapon(eWeaponType weapon, bool shoots) const;

    bool CanStartSlideOnCarState(Pedestrian* pedestrian) const;

    void SetInCarPositionToDoor(Pedestrian* pedestrian);
    void SetInCarPositionToSeat(Pedestrian* pedestrian);

private:
    const ePedestrianState mStateIdentifier;
};

// base class of idle state
class PedestrianStateIdleBase: public PedestrianBaseState
{
protected:
    PedestrianStateIdleBase(ePedestrianState stateIdentifier) : PedestrianBaseState(stateIdentifier) {}
    void ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;
    bool ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent) override;

protected:
    bool TryToShoot(Pedestrian* pedestrian);

    PedestrianBaseState* GetNextIdleState(Pedestrian* pedestrian, Timespan deltaTime);
};

// process state ePedestrianState_StandingStill
class PedestrianStateStandingStill: public PedestrianStateIdleBase
{
public:
    PedestrianStateStandingStill() : PedestrianStateIdleBase(ePedestrianState_StandingStill) {}
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    bool ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent) override;
};

// process state ePedestrianState_Walks
class PedestrianStateWalks: public PedestrianStateIdleBase
{
public:
    PedestrianStateWalks() : PedestrianStateIdleBase(ePedestrianState_Walks) {}
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    bool ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent) override;
};

// process state ePedestrianState_Runs
class PedestrianStateRuns: public PedestrianStateIdleBase
{
public:
    PedestrianStateRuns() : PedestrianStateIdleBase(ePedestrianState_Runs) {}
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    bool ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent) override;
};

// process state ePedestrianState_StandsAndShoots
class PedestrianStateStandsAndShoots: public PedestrianStateIdleBase
{
public:
    PedestrianStateStandsAndShoots() : PedestrianStateIdleBase(ePedestrianState_StandsAndShoots) {}
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    bool ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent) override;
};

// process state ePedestrianState_WalksAndShoots
class PedestrianStateWalksAndShoots: public PedestrianStateIdleBase
{
public:
    PedestrianStateWalksAndShoots() : PedestrianStateIdleBase(ePedestrianState_WalksAndShoots) {}
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    bool ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent) override;
};

// process state ePedestrianState_RunsAndShoots
class PedestrianStateRunsAndShoots: public PedestrianStateIdleBase
{
public:
    PedestrianStateRunsAndShoots() : PedestrianStateIdleBase(ePedestrianState_RunsAndShoots) {}
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    bool ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent) override;
};

// process state ePedestrianState_Falling
class PedestrianStateFalling: public PedestrianBaseState
{
public:
    PedestrianStateFalling() : PedestrianBaseState(ePedestrianState_Falling) {}
    void ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    void ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
};

// process state ePedestrianState_KnockedDown
class PedestrianStateKnockedDown: public PedestrianBaseState
{
public:
    PedestrianStateKnockedDown() : PedestrianBaseState(ePedestrianState_KnockedDown) {}
    void ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
};

// process state ePedestrianState_SlideOnCar
class PedestrianStateSlideOnCar: public PedestrianBaseState
{
public:
    PedestrianStateSlideOnCar() : PedestrianBaseState(ePedestrianState_SlideOnCar) {}
    void ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
protected:
    void ProcessRotateActions(Pedestrian* pedestrian, Timespan deltaTime) override;
    void ProcessMotionActions(Pedestrian* pedestrian, Timespan deltaTime) override;
};

// process state ePedestrianState_EnteringCar
class PedestrianStateEnterCar: public PedestrianBaseState
{
public:
    PedestrianStateEnterCar() : PedestrianBaseState(ePedestrianState_EnteringCar) {}
    void ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
};

// process state ePedestrianState_ExitingCar
class PedestrianStateExitCar: public PedestrianBaseState
{
public:
    PedestrianStateExitCar() : PedestrianBaseState(ePedestrianState_ExitingCar) {}
    void ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    void ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
};

// process state ePedestrianState_DrivingCar
class PedestrianStateDrivingCar: public PedestrianBaseState
{
public:
    PedestrianStateDrivingCar() : PedestrianBaseState(ePedestrianState_DrivingCar) {}
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    void ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    bool ProcessStateEvent(Pedestrian* pedestrian, const PedestrianStateEvent& stateEvent) override;
};

// process state ePedestrianState_Dead
class PedestrianStateDead: public PedestrianBaseState
{
public:
    PedestrianStateDead() : PedestrianBaseState(ePedestrianState_Dead) {}
    void ProcessStateFrame(Pedestrian* pedestrian, Timespan deltaTime) override;
    void ProcessStateEnter(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
    void ProcessStateExit(Pedestrian* pedestrian, const PedestrianStateEvent* transitionEvent) override;
};
