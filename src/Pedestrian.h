#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"
#include "CharacterController.h"
#include "GameObject.h"

class SpriteBatch;

// defines generic city pedestrian
class Pedestrian final: public GameObject
{
    friend class GameObjectsManager;

    // all base states should have access to private data
    friend class PedestrianBaseState;
    friend class PedestrianStateIdle;
    friend class PedestrianStateIdleShoots;
    friend class PedestrianStateFalling;
    friend class PedestrianStateEnterOrExitCar;
    friend class PedestrianStateSlideOnCar;

public:
    // public for convenience, should not be modified directly
    CharacterController* mController; // controls pedestrian actions
    PedPhysicsComponent* mPhysicsComponent;

    ePedestrianState mCurrentStateID;
    Timespan mCurrentStateTime; // time since current state has started

    bool mCtlActions[ePedestrianAction_COUNT]; // control actions

    // inventory
    eWeaponType mCurrentWeapon;
    int mWeaponsAmmo[eWeaponType_COUNT]; // -1 means infinite, 'fists' is good example

public:
    // @param id: Unique object identifier, constant
    Pedestrian(unsigned int id);
    ~Pedestrian();

    // setup initial state when spawned on level
    void EnterTheGame();

    void UpdateFrame(Timespan deltaTime);
    void DrawFrame(SpriteBatch& spriteBatch);

    // set position for pedestrian, does nothing if sitting in car
    // @param position: World position
    void SetPosition(const glm::vec3& position);

    // set sign direction for pedestrian, does nothing if sitting in car
    // @param rotationAngle: Angle value
    void SetHeading(cxx::angle_t rotationAngle);

    // set current weapon, does nothing if out of ammo
    void ChangeWeapon(eWeaponType newWeapon);

private:
    void SetCurrentState(ePedestrianState newStateID, bool isInitial);
    void SetAnimation(eSpriteAnimationID animation, eSpriteAnimLoop loopMode);
    float ComputeDrawHeight(const glm::vec3& position, cxx::angle_t rotationAngle);

private:
    eSpriteAnimationID mCurrentAnimID;
    SpriteAnimation mCurrentAnimState;

    Sprite mDrawSprite;

    // internal stuff that can be touched only by PedestrianManager
    cxx::intrusive_node<Pedestrian> mActivePedsNode;
    cxx::intrusive_node<Pedestrian> mDeletePedsNode;
};
