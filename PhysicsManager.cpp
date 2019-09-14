#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapData.h"

PhysicsManager gPhysics;

PhysicsManager::PhysicsManager()
    : mPhysicsWorld()
{
    mDebugDraw.SetFlags(0);
}

bool PhysicsManager::Initialize()
{
    b2Vec2 gravity {0.0f, 0.0f}; // default gravity shoild be disabled

    mPhysicsWorld = new b2World(gravity);
    mPhysicsWorld->SetDebugDraw(&mDebugDraw);

    b2BodyDef myBodyDef;
    myBodyDef.type = b2_dynamicBody; //this will be a dynamic body
    myBodyDef.position.Set(91.0f, 236.0f); //set the starting position
    myBodyDef.angle = 0; //set the starting angle

    b2Body* dynamicBody = mPhysicsWorld->CreateBody(&myBodyDef);

    b2CircleShape circleShape;
    circleShape.m_p.Set(0, 0); //position, relative to body position
    circleShape.m_radius = 0.1f; //radius

    b2FixtureDef boxFixtureDef;
    boxFixtureDef.shape = &circleShape;
    boxFixtureDef.density = 1.0f;
    dynamicBody->CreateFixture(&boxFixtureDef);


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

    mPhysicsWorld->DrawDebugData();
}

void PhysicsManager::EnableDebugDraw(bool isEnabled)
{
    if (isEnabled)
    {
        mDebugDraw.SetFlags(b2Draw::e_shapeBit);
    }
    else
    {
        mDebugDraw.SetFlags(0);
    }
}
