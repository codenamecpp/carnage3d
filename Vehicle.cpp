#include "stdafx.h"
#include "Vehicle.h"
#include "PhysicsManager.h"
#include "PhysicsComponents.h"
#include "GameMapManager.h"

Vehicle::Vehicle(unsigned int id)
    : GameObject(id)
    , mActiveCarsNode(this)
    , mDeleteCarsNode(this)
    , mPhysicsComponent()
    , mDead()
    , mCarStyle()
    , mMarkForDeletion()
{
}

Vehicle::~Vehicle()
{
    if (mPhysicsComponent)
    {
        gPhysics.DestroyPhysicsComponent(mPhysicsComponent);
    }
}

void Vehicle::EnterTheGame()
{
    debug_assert(mCarStyle);

    glm::vec3 startPosition;
    
    mPhysicsComponent = gPhysics.CreateCarPhysicsComponent(startPosition, 0.0f, mCarStyle);
    debug_assert(mPhysicsComponent);

    mMarkForDeletion = false;
    mDead = false;
}

void Vehicle::UpdateFrame(Timespan deltaTime)
{
}