#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"
#include "CharacterController.h"
#include "GameObject.h"
#include "Sprite2D.h"
#include "PedestrianStates.h"

class SpriteBatch;
class DebugRenderer;

// defines generic city pedestrian
class Pedestrian final: public GameObject
{
public:
    // public for convenience, should not be modified directly
    CharacterController* mController; // controls pedestrian actions
    PedPhysicsComponent* mPhysicsComponent;

    Timespan mCurrentStateTime; // time since current state has started
    Timespan mWeaponRechargeTime; // next time weapon can be used again

    bool mCtlActions[ePedestrianAction_COUNT]; // control actions

    float mDrawHeight;
    int mRemapIndex;
    
    // in car
    Vehicle* mCurrentCar = nullptr;
    eCarSeat mCurrentSeat;

    // inventory
    eWeaponType mCurrentWeapon;
    int mWeaponsAmmo[eWeaponType_COUNT]; // -1 means infinite, 'fists' is good example

public:
    // @param id: Unique object identifier, constant
    Pedestrian(GameObjectID id);
    ~Pedestrian();

    // setup initial state when spawned on level
    void EnterTheGame();

    void UpdateFrame(Timespan deltaTime) override;
    void DrawFrame(SpriteBatch& spriteBatch) override;
    void DrawDebug(DebugRenderer& debugRender) override;

    // send event to pedestrian, it may be ignored depending on its current state
    // @returns false if event not processed
    bool ProcessEvent(const PedestrianStateEvent& eventData);

    // check if pedestrian entering/exiting or driving car at this moment
    bool IsCarPassenger() const;
    bool IsCarDriver() const;

    // check if pedestrian is in specific state
    bool IsStanding() const;
    bool IsShooting() const;
    bool IsWalking() const;
    bool IsUnconscious() const;

    // detects identifier of current pedestrian state
    ePedestrianState GetCurrentStateID() const;

private:
    // changes current pedestrian state, note that switching to the same state will be ignored
    // some of the states must be configured before switch
    // @param nextState: New state, cannot be null
    // @param transitionEvent: Transition event, it will be handled by new state on enter, optional
    void ChangeState(PedestrianBaseState* nextState, const PedestrianStateEvent* transitionEvent);

    void SetAnimation(eSpriteAnimID animation, eSpriteAnimLoop loopMode);
    void ComputeDrawHeight(const glm::vec3& position);

    void SetCurrentWeapon(eWeaponType weapon);

    void SetCarEntered(Vehicle* targetCar, eCarSeat targetSeat);
    void SetCarExited();

private:
    friend class GameObjectsManager;

    // all base states should have access to private data
    friend class PedestrianStateIdleBase;
    friend class PedestrianStateStandingStill;
    friend class PedestrianStateWalks;
    friend class PedestrianStateRuns;
    friend class PedestrianStateStandsAndShoots;
    friend class PedestrianStateWalksAndShoots;
    friend class PedestrianStateRunsAndShoots;
    friend class PedestrianStateFalling;
    friend class PedestrianStateEnterCar;
    friend class PedestrianStateExitCar;
    friend class PedestrianStateSlideOnCar;
    friend class PedestrianStateDrivingCar;
    friend class PedestrianStateKnockedDown;
    friend class PedestrianStateDead;

    eSpriteAnimID mCurrentAnimID;
    SpriteAnimation mCurrentAnimState;
    PedestrianBaseState* mCurrentState = nullptr;

    Sprite2D mDrawSprite;

    // all pedestrian states
    PedestrianStateStandingStill mStateStandingStill;
    PedestrianStateWalks mStateWalks;
    PedestrianStateRuns mStateRuns;
    PedestrianStateStandsAndShoots mStateStandsAndShoots;
    PedestrianStateWalksAndShoots mStateWalksAndShoots;
    PedestrianStateRunsAndShoots mStateRunsAndShoots;
    PedestrianStateFalling mStateFalling;
    PedestrianStateEnterCar mStateEnterCar;
    PedestrianStateExitCar mStateExitCar;
    PedestrianStateSlideOnCar mStateSlideOnCar;
    PedestrianStateDrivingCar mStateDrivingCar;
    PedestrianStateKnockedDown mStateKnockedDown;
    PedestrianStateDead mStateDead;

    // internal stuff that can be touched only by PedestrianManager
    cxx::intrusive_node<Pedestrian> mPedsListNode;
};

const int Sizeof_Pedestrian = sizeof(Pedestrian);