#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"
#include "GameObject.h"

class SpriteBatch;

// defines vehicle instance
class Vehicle final: public GameObject
{
    friend class GameObjectsManager;

public:
    // public for convenience, should not be modified directly
    CarPhysicsComponent* mPhysicsComponent;

    bool mDead;

    CarStyle* mCarStyle; // cannot be null

public:
    // @param id: Unique object identifier, constant
    Vehicle(GameObjectID_t id);
    ~Vehicle();

    // setup initial state when spawned on level
    void EnterTheGame();

    void UpdateFrame(Timespan deltaTime);
    void DrawFrame(SpriteBatch& spriteBatch);

private:
    float ComputeDrawHeight(const glm::vec3& position, cxx::angle_t rotationAngle);
    SpriteDeltaBits_t GetSpriteDeltas() const;

private:
    Sprite mChassisSprite;

    // internal stuff that can be touched only by CarsManager
    cxx::intrusive_node<Vehicle> mActiveCarsNode;
    cxx::intrusive_node<Vehicle> mDeleteCarsNode;
};