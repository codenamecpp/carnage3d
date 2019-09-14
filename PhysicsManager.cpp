#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapData.h"

PhysicsManager gPhysics;

PhysicsManager::PhysicsManager()
    : mPhysicsWorld()
    , mMapPhysicsBody()
{
    mDebugDraw.SetFlags(0);
}

bool PhysicsManager::Initialize()
{
    b2Vec2 gravity {0.0f, 0.0f}; // default gravity shoild be disabled
    mPhysicsWorld = new b2World(gravity);
    mPhysicsWorld->SetDebugDraw(&mDebugDraw);
    mPhysicsWorld->SetContactFilter(this);

    // create collsition body for map
    mMapPhysicsBody = CreateMapBody();
    debug_assert(mMapPhysicsBody);

    return true;
}

void PhysicsManager::Deinit()
{
    if (mMapPhysicsBody)
    {
        DestroyPhysicsObject(mMapPhysicsBody);
        mMapPhysicsBody = nullptr;
    }
    SafeDelete(mPhysicsWorld);
}

void PhysicsManager::UpdateFrame(Timespan deltaTime)
{
    int maxSimulationStepsPerFrame = 3;
    int numSimulations = 0;

    const float simulationStepF = 1.0f / 60.0f;
    const int velocityIterations = 3; // recommended 8
    const int positionIterations = 3; // recommended 3

    mSimulationTimeAccumulator += deltaTime.ToSeconds();
    while (mSimulationTimeAccumulator > simulationStepF)
    {
        if (++numSimulations > maxSimulationStepsPerFrame)
        {
            mSimulationTimeAccumulator = 0.0f;
            break;
        }
        mSimulationTimeAccumulator -= simulationStepF;
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

bool PhysicsManager::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
    return true;
}

PhysicsObject* PhysicsManager::CreatePedestrianBody(const glm::vec3& position, float angleDegrees)
{
    PhysicsObject* physicsObject = mObjectsPool.create();
    physicsObject->mPhysicsWorld = mPhysicsWorld;

    // create body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {position.x, position.y};
    bodyDef.angle = glm::radians(angleDegrees);
    bodyDef.fixedRotation = true;
    bodyDef.userData = physicsObject;

    physicsObject->mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(physicsObject->mPhysicsBody);
    physicsObject->mDepth = 0.5f;
    physicsObject->mZCoord = position.z;
    
    b2CircleShape shapeDef;
    shapeDef.m_radius = PHYSICS_PED_BOUNDING_SPHERE_RADIUS;

    b2Fixture* b2fixture = physicsObject->mPhysicsBody->CreateFixture(&shapeDef, 10.0f);
    debug_assert(b2fixture);

    return physicsObject;
}

PhysicsObject* PhysicsManager::CreateMapBody()
{
    // build object for layer 1

    PhysicsObject* physicsObject = mObjectsPool.create();
    physicsObject->mPhysicsWorld = mPhysicsWorld;

    // create body
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.userData = physicsObject;

    physicsObject->mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(physicsObject->mPhysicsBody);
    physicsObject->mDepth = 1.0f;
    physicsObject->mZCoord = 1.0f;

    // for each block create fixture
    for (int x = 0; x < MAP_DIMENSIONS; ++x)
    for (int y = 0; y < MAP_DIMENSIONS; ++y)
    {
        MapCoord currentMapCoord { x, y, 1 };
        BlockStyleData* blockData = gGameMap.GetBlock(currentMapCoord);
        debug_assert(blockData);

        if (blockData->mGroundType != eGroundType_Building)
            continue;

        b2PolygonShape b2shapeDef;
        b2Vec2 center { 
            (x * MAP_BLOCK_LENGTH) + (MAP_BLOCK_LENGTH * 0.5f), 
            (y * MAP_BLOCK_LENGTH) + (MAP_BLOCK_LENGTH * 0.5f)
        };
        b2shapeDef.SetAsBox(MAP_BLOCK_LENGTH * 0.5f, MAP_BLOCK_LENGTH * 0.5f, center, 0.0f);
        b2Fixture* b2fixture = physicsObject->mPhysicsBody->CreateFixture(&b2shapeDef, 1.0f);
        debug_assert(b2fixture);
    }
    return physicsObject;
}

void PhysicsManager::DestroyPhysicsObject(PhysicsObject* object)
{
    debug_assert(object);

    mObjectsPool.destroy(object);
}