#pragma once

#include "GameDefs.h"

enum ePedestrianStateEvent
{
    ePedestrianStateEvent_None, // dummy event

    ePedestrianStateEvent_Spawn,
    ePedestrianStateEvent_EnterCar,
    ePedestrianStateEvent_ExitCar,
    ePedestrianStateEvent_DamageFromWeapon,
    ePedestrianStateEvent_PullOutFromCar,
    ePedestrianStateEvent_Die,
    ePedestrianStateEvent_FallFromHeightStart,
    ePedestrianStateEvent_FallFromHeightEnd,
    ePedestrianStateEvent_WeaponChange, 
    ePedestrianStateEvent_WaterContact,
};

// defines state event
struct PedestrianStateEvent
{
public:
    PedestrianStateEvent(ePedestrianStateEvent eventID): mID(eventID)
    {
    }
public:
    const ePedestrianStateEvent mID;

    Vehicle* mTargetCar = nullptr;
    eCarSeat mTargetSeat;

    Pedestrian* mAttacker = nullptr;
    eWeaponType mWeaponType;

    ePedestrianDeathReason mDeathReason;
};

//////////////////////////////////////////////////////////////////////////

class PedestrianStatesManager: public cxx::noncopyable
{
public:
    PedestrianStatesManager(Pedestrian* pedestrian);

    // get current state identifier
    inline ePedestrianState GetCurrentStateID() const { return mCurrentStateID; }

    // changes current pedestrian state, note that switching to the same state will be ignored
    // @param nextState: New state
    // @param transitionEvent: Transition event, it will be handled by new state on enter
    void ChangeState(ePedestrianState nextState, const PedestrianStateEvent& evData);

    // send event to current state
    bool ProcessEvent(const PedestrianStateEvent& evData);

    // update current state
    void ProcessFrame(Timespan deltaTime);

private:
    void InitFuncsTable();
    
    // state helpers
    void ProcessRotateActions(Timespan deltaTime);
    void ProcessMotionActions(Timespan deltaTime);
    bool TryToShoot();

    ePedestrianState GetNextIdleState(Timespan deltaTime);

    // anims helpers
    eSpriteAnimID DetectIdleAnimation() const;

    bool CanStartSlideOnCarState() const;
    void SetInCarPositionToDoor();
    void SetInCarPositionToSeat();

    // state unspecified
    void StateDummy_ProcessFrame(Timespan deltaTime) {}
    void StateDummy_ProcessEnter(const PedestrianStateEvent& stateEvent) {}
    void StateDummy_ProcessExit() {}
    bool StateDummy_ProcessEvent(const PedestrianStateEvent& stateEvent) { return false; }

    // state dead
    void StateDead_ProcessEnter(const PedestrianStateEvent& stateEvent);

    // state driving car
    void StateDriveCar_ProcessEnter(const PedestrianStateEvent& stateEvent);
    bool StateDriveCar_ProcessEvent(const PedestrianStateEvent& stateEvent);

    // state exit car
    void StateExitCar_ProcessFrame(Timespan deltaTime);
    void StateExitCar_ProcessEnter(const PedestrianStateEvent& stateEvent);
    void StateExitCar_ProcessExit();

    // state enter car
    void StateEnterCar_ProcessFrame(Timespan deltaTime);
    void StateEnterCar_ProcessEnter(const PedestrianStateEvent& stateEvent);

    // state slide on car
    void StateSlideCar_ProcessFrame(Timespan deltaTime);
    void StateSlideCar_ProcessEnter(const PedestrianStateEvent& stateEvent);
    bool StateSlideCar_ProcessEvent(const PedestrianStateEvent& stateEvent);

    // state knocked down
    void StateKnockedDown_ProcessFrame(Timespan deltaTime);
    void StateKnockedDown_ProcessEnter(const PedestrianStateEvent& stateEvent);
    bool StateKnockedDown_ProcessEvent(const PedestrianStateEvent& stateEvent);

    // state falling
    void StateFalling_ProcessEnter(const PedestrianStateEvent& stateEvent);
    void StateFalling_ProcessExit();
    bool StateFalling_ProcessEvent(const PedestrianStateEvent& stateEvent);

    // states standing, walking, running, shooting
    void StateIdle_ProcessFrame(Timespan deltaTime);
    void StateIdle_ProcessEnter(const PedestrianStateEvent& stateEvent);
    bool StateIdle_ProcessEvent(const PedestrianStateEvent& stateEvent);

    // states drowning
    void StateDrowning_ProcessFrame(Timespan deltaTime);
    void StateDrowning_ProcessEnter(const PedestrianStateEvent& stateEvent);

private:

    struct StateFuncs
    {
        void (PedestrianStatesManager::*pfStateEnter)(const PedestrianStateEvent& stateEvent);
        void (PedestrianStatesManager::*pfStateExit)();
        void (PedestrianStatesManager::*pfStateFrame)(Timespan deltaTime);
        bool (PedestrianStatesManager::*pfStateEvent)(const PedestrianStateEvent& stateEvent);
    };

    Pedestrian* mPedestrian;
    ePedestrianState mCurrentStateID = ePedestrianState_Unspecified;
    StateFuncs mFuncsTable[ePedestrianState_COUNT];
};

