#pragma once

#include "GameDefs.h"
#include "DamageInfo.h"

enum ePedestrianStateEvent
{
    ePedestrianStateEvent_None, // dummy event

    ePedestrianStateEvent_Spawn,
    ePedestrianStateEvent_EnterCar,
    ePedestrianStateEvent_ExitCar,
    ePedestrianStateEvent_ReceiveDamage,
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
    ePedestrianStateEvent mID;

    Vehicle* mTargetCar = nullptr;
    eCarSeat mTargetSeat;

    DamageInfo mDamageInfo; // specified for ReceiveDamage event

    ePedestrianDeathReason mDeathReason; // specified for Die event
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
    void ProcessFrame();
    void ProcessSimulationFrame();

    bool CanStartSlideOnCarState() const;

private:
    void InitFuncsTable();
    
    // state helpers
    void ProcessRotateActions();

    ePedestrianState GetNextIdleState();

    // anims helpers
    ePedestrianAnimID DetectIdleAnimation(bool isShooting) const;

    void SetInCarPositionToDoor();
    void SetInCarPositionToSeat();

    bool TryProcessDamage(const DamageInfo& damageInfo);

    // state unspecified
    void StateDummy_ProcessFrame() {}
    void StateDummy_ProcessEnter(const PedestrianStateEvent& stateEvent) {}
    void StateDummy_ProcessExit() {}
    bool StateDummy_ProcessEvent(const PedestrianStateEvent& stateEvent) { return false; }
    void StateDummy_ProcessSimulationFrame() {}

    // state dead
    void StateDead_ProcessEnter(const PedestrianStateEvent& stateEvent);

    // state driving car
    void StateDriveCar_ProcessEnter(const PedestrianStateEvent& stateEvent);
    void StateDriveCar_ProcessExit();
    bool StateDriveCar_ProcessEvent(const PedestrianStateEvent& stateEvent);

    // state exit car
    void StateExitCar_ProcessFrame();
    void StateExitCar_ProcessEnter(const PedestrianStateEvent& stateEvent);
    void StateExitCar_ProcessExit();

    // state enter car
    void StateEnterCar_ProcessFrame();
    void StateEnterCar_ProcessEnter(const PedestrianStateEvent& stateEvent);

    // state slide on car
    void StateSlideCar_ProcessFrame();
    void StateSlideCar_ProcessEnter(const PedestrianStateEvent& stateEvent);
    bool StateSlideCar_ProcessEvent(const PedestrianStateEvent& stateEvent);

    // state knocked down
    void StateStunned_ProcessFrame();
    void StateStunned_ProcessEnter(const PedestrianStateEvent& stateEvent);
    bool StateStunned_ProcessEvent(const PedestrianStateEvent& stateEvent);

    // state falling
    void StateFalling_ProcessEnter(const PedestrianStateEvent& stateEvent);
    void StateFalling_ProcessExit();
    bool StateFalling_ProcessEvent(const PedestrianStateEvent& stateEvent);

    // states standing, walking, running, shooting
    void StateIdle_ProcessFrame();
    void StateIdle_ProcessEnter(const PedestrianStateEvent& stateEvent);
    bool StateIdle_ProcessEvent(const PedestrianStateEvent& stateEvent);

    // states drowning
    void StateDrowning_ProcessFrame();
    void StateDrowning_ProcessEnter(const PedestrianStateEvent& stateEvent);

    // state electrocuted, die
    void StateElectrocuted_ProcessFrame();
    void StateElectrocuted_ProcessEnter(const PedestrianStateEvent& stateEvent);

private:

    struct StateFuncs
    {
        using PFStateEnter = void (PedestrianStatesManager::*)(const PedestrianStateEvent& stateEvent);
        PFStateEnter pfStateEnter = nullptr;

        using PFStateExit = void (PedestrianStatesManager::*)();
        PFStateExit pfStateExit = nullptr;

        using PFStateFrame = void (PedestrianStatesManager::*)();
        PFStateFrame pfStateFrame = nullptr;

        using PFStateEvent = bool (PedestrianStatesManager::*)(const PedestrianStateEvent& stateEvent);
        PFStateEvent pfStateEvent = nullptr;

        using PFStateSimulationFrame = void (PedestrianStatesManager::*)();
        PFStateSimulationFrame pfStateSimulationFrame = nullptr;

        StateFuncs(PFStateEnter _pfEnter, PFStateExit _pfExit, PFStateFrame _pfFrame, PFStateEvent _pfEvent, PFStateSimulationFrame _pfSimulationFrame)
            : pfStateEnter(_pfEnter)
            , pfStateExit(_pfExit)
            , pfStateFrame(_pfFrame)
            , pfStateEvent(_pfEvent)
            , pfStateSimulationFrame(_pfSimulationFrame)
        {
        }
        StateFuncs() = default;
    };

    Pedestrian* mPedestrian;
    ePedestrianState mCurrentStateID = ePedestrianState_Unspecified;
    StateFuncs mFuncsTable[ePedestrianState_COUNT];
};

