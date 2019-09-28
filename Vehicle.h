#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"

// defines vehicle instance
class Vehicle final: public cxx::noncopyable
{
    friend class GameObjectsManager;

public:
    // public for convenience, should not be modified directly
    const unsigned int mID; // unique identifier

    CarPhysicsComponent* mPhysicsComponent;
    bool mDead;
    bool mMarkForDeletion;

    CarStyle* mCarStyle; // cannot be null

public:
    // @param id: Unique object identifier, constant
    Vehicle(unsigned int id);
    ~Vehicle();

    // setup initial state when spawned on level
    void EnterTheGame();
    void UpdateFrame(Timespan deltaTime);

private:
    // internal stuff that can be touched only by CarsManager
    cxx::intrusive_node<Vehicle> mActiveCarsNode;
    cxx::intrusive_node<Vehicle> mDeleteCarsNode;
};