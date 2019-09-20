#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "GameCheatsWindow.h"

//////////////////////////////////////////////////////////////////////////

union b2FixtureData_map
{
    b2FixtureData_map(void* asPointer = nullptr)
        : mAsPointer(asPointer)
    {
    }

    struct
    {
        unsigned char mX, mZ;
    };

    void* mAsPointer;
};

static_assert(sizeof(b2FixtureData_map) <= sizeof(void*), "Cannot pack data into pointer");

//////////////////////////////////////////////////////////////////////////

PhysicsManager gPhysics;

PhysicsManager::PhysicsManager()
    : mPhysicsWorld()
    , mMapCollisionBody()
{
    mDebugDraw.SetFlags(0);
}

bool PhysicsManager::Initialize()
{
    b2Vec2 gravity {0.0f, 0.0f}; // default gravity shoild be disabled
    mPhysicsWorld = new b2World(gravity);
    mPhysicsWorld->SetDebugDraw(&mDebugDraw);
    mPhysicsWorld->SetContactListener(this);

    // create collsition body for map
    CreateMapCollisionBody();
    return true;
}

void PhysicsManager::Deinit()
{
    if (mMapCollisionBody)
    {
        DestroyPhysicsObject(mMapCollisionBody);
        mMapCollisionBody = nullptr;
    }
    SafeDelete(mPhysicsWorld);
}

void PhysicsManager::UpdateFrame(Timespan deltaTime)
{
    int maxSimulationStepsPerFrame = 5;
    int numSimulations = 0;

    const int velocityIterations = 3; // recommended 8
    const int positionIterations = 2; // recommended 3

    mSimulationTimeAccumulator += deltaTime.ToSeconds();

    while (mSimulationTimeAccumulator >= PHYSICS_SIMULATION_STEP)
    {
        mSimulationTimeAccumulator -= PHYSICS_SIMULATION_STEP;
        mPhysicsWorld->Step(PHYSICS_SIMULATION_STEP, velocityIterations, positionIterations);
        if (++numSimulations == maxSimulationStepsPerFrame)
        {
            break;
        }
        UpdatePedsGravity();
    }
    mPhysicsWorld->DrawDebugData();
}

PhysicsObject* PhysicsManager::CreatePedestrianBody(const glm::vec3& position, float angleDegrees)
{
    PhysicsObject* physicsObject = mObjectsPool.create();
    physicsObject->mPhysicsWorld = mPhysicsWorld;

    // create body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {position.x, position.z};
    bodyDef.angle = glm::radians(angleDegrees);
    bodyDef.fixedRotation = true;
    bodyDef.userData = physicsObject;

    physicsObject->mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(physicsObject->mPhysicsBody);
    physicsObject->mDepth = 0.5f;
    physicsObject->mHeight = position.y;
    
    b2CircleShape shapeDef;
    shapeDef.m_radius = PHYSICS_PED_BOUNDING_SPHERE_RADIUS;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 10.0f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_PED;

    b2Fixture* b2fixture = physicsObject->mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);

    return physicsObject;
}

void PhysicsManager::CreateMapCollisionBody()
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
    physicsObject->mDepth = MAP_LAYERS_COUNT * MAP_BLOCK_LENGTH;
    physicsObject->mHeight = 0.0f;

    int numFixtures = 0;
    // for each block create fixture
    for (int x = 0; x < MAP_DIMENSIONS; ++x)
    for (int y = 0; y < MAP_DIMENSIONS; ++y)
    {
        for (int layer = 0; layer < MAP_LAYERS_COUNT; ++layer)
        {
            BlockStyleData* blockData = gGameMap.GetBlock(x, y, layer);
            debug_assert(blockData);

            if (blockData->mGroundType != eGroundType_Building)
                continue;

            // checek blox is inner
            {
                BlockStyleData* neighbourE = gGameMap.GetBlockClamp(x + 1, y, layer); 
                BlockStyleData* neighbourW = gGameMap.GetBlockClamp(x - 1, y, layer); 
                BlockStyleData* neighbourN = gGameMap.GetBlockClamp(x, y - 1, layer); 
                BlockStyleData* neighbourS = gGameMap.GetBlockClamp(x, y + 1, layer);

                auto is_walkable = [](eGroundType gtype)
                {
                    return gtype == eGroundType_Field || gtype == eGroundType_Field || gtype == eGroundType_Pawement || gtype == eGroundType_Road;
                };

                if (!is_walkable(neighbourE->mGroundType) && !is_walkable(neighbourW->mGroundType) &&
                    !is_walkable(neighbourN->mGroundType) && !is_walkable(neighbourS->mGroundType))
                {
                    continue; // just ignore this block 
                }
            }

            b2PolygonShape b2shapeDef;
            b2Vec2 center { 
                (x * MAP_BLOCK_LENGTH) + (MAP_BLOCK_LENGTH * 0.5f), 
                (y * MAP_BLOCK_LENGTH) + (MAP_BLOCK_LENGTH * 0.5f)
            };
            b2shapeDef.SetAsBox(MAP_BLOCK_LENGTH * 0.5f, MAP_BLOCK_LENGTH * 0.5f, center, 0.0f);

            b2FixtureData_map fixtureData;
            fixtureData.mX = x;
            fixtureData.mZ = y;

            b2FixtureDef b2fixtureDef;
            b2fixtureDef.density = 1.0f;
            b2fixtureDef.shape = &b2shapeDef;
            b2fixtureDef.userData = fixtureData.mAsPointer;
            b2fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_BUILDING;

            b2Fixture* b2fixture = physicsObject->mPhysicsBody->CreateFixture(&b2fixtureDef);
            debug_assert(b2fixture);

            ++numFixtures;
            break; // single fixture per block column
        }
    }

    mMapCollisionBody = physicsObject;
}

void PhysicsManager::DestroyPhysicsObject(PhysicsObject* object)
{
    debug_assert(object);

    mObjectsPool.destroy(object);
}

void PhysicsManager::BeginContact(b2Contact* contact)
{
}

void PhysicsManager::EndContact(b2Contact* contact)
{
}

void PhysicsManager::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
}

void PhysicsManager::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
}

void PhysicsManager::UpdatePedsGravity()
{
    for (Pedestrian* currPedestrian: gCarnageGame.mPedsManager.mActivePedsList)
    {
        // correct y coord on slopes
        glm::vec3 pedestrianPos = currPedestrian->mPhysicalBody->GetPosition();
        pedestrianPos.y = gGameMap.GetHeightAtPosition(pedestrianPos);
        currPedestrian->SetPosition(pedestrianPos);
    }
}