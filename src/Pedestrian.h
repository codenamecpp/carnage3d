#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"
#include "CharacterController.h"
#include "GameObject.h"
#include "Sprite2D.h"
#include "PedestrianStates.h"

class SpriteBatch;

// defines generic city pedestrian
class Pedestrian final: public GameObject
{
public:
    // public for convenience, should not be modified directly
    CharacterController* mController; // controls pedestrian actions
    PedPhysicsComponent* mPhysicsComponent;

    Timespan mCurrentStateTime; // time since current state has started

    bool mCtlActions[ePedestrianAction_COUNT]; // control actions

    float mDrawHeight;
    
    // in car
    Vehicle* mCurrentCar = nullptr;
    eCarSeat mCurrentSeat;

    // inventory
    eWeaponType mCurrentWeapon;
    int mWeaponsAmmo[eWeaponType_COUNT]; // -1 means infinite, 'fists' is good example

public:
    // @param id: Unique object identifier, constant
    Pedestrian(GameObjectID_t id);
    ~Pedestrian();

    // setup initial state when spawned on level
    void EnterTheGame();

    void UpdateFrame(Timespan deltaTime);
    void DrawFrame(SpriteBatch& spriteBatch);

    // set current weapon, does nothing if out of ammo
    void ChangeWeapon(eWeaponType newWeapon);

    // pedestrian will try enter vehicle, may fail depending on its current state
    // @param targetCar: Car
    // @param targetSeat: Seat, cannot be 'any'
    void TakeSeatInCar(Vehicle* targetCar, eCarSeat targetSeat);

    // pedestrian will try exit vehicle, may fail depending on its current state
    void LeaveCar();

    bool IsDrivingCar() const;

    // detects identifier of current pedestrian state
    ePedestrianState GetCurrentStateID() const;

private:
    // changes current pedestrian state, note that switching to the same state will be ignored
    // some of the states must be configured before switch
    // @param nextState: New state, cannot be null
    // @param transitionEvent: Transition event, it will be handled by new state on enter, optional
    void ChangeState(PedestrianBaseState* nextState, const PedestrianStateEvent* transitionEvent);

    void SetAnimation(eSpriteAnimationID animation, eSpriteAnimLoop loopMode);
    void ComputeDrawHeight(const glm::vec3& position);

    void HandleCarEntered();
    void HandleCarExited();

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

    eSpriteAnimationID mCurrentAnimID;
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

    // internal stuff that can be touched only by PedestrianManager
    cxx::intrusive_node<Pedestrian> mActivePedsNode;
    cxx::intrusive_node<Pedestrian> mDeletePedsNode;
};

const int Sizeof_Pedestrian = sizeof(Pedestrian);