#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapData.h"

PhysicsManager gPhysics;

PhysicsManager::PhysicsManager()
    : mPhysicsWorld()
{
}

bool PhysicsManager::Initialize()
{
    b2Vec2 gravity {0.0f, 0.0f}; // default gravity shoild be disabled

    mPhysicsWorld = new b2World(gravity);
    mPhysicsWorld->SetDebugDraw(&mDebugDraw);

    return true;
}

void PhysicsManager::Deinit()
{
    SafeDelete(mPhysicsWorld);
}

void PhysicsManager::UpdateFrame(Timespan deltaTime)
{
    const float simulationStepF = 1.0f / 60.0f;
    const int velocityIterations = 8; // recommended 8
    const int positionIterations = 3; // recommended 3

    static const Timespan simulationStep = Timespan::FromSeconds(simulationStepF);

    mTimeSinceLastSimulation += deltaTime;
    while (mTimeSinceLastSimulation > simulationStep)
    {
        mTimeSinceLastSimulation -= simulationStep;
        mPhysicsWorld->Step(simulationStepF, velocityIterations, positionIterations);
    }
}