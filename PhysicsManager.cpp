#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapData.h"
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
        unsigned char mX, mY;
    };

    void* mAsPointer;
};

static_assert(sizeof(b2FixtureData_map) <= sizeof(void*), "Cannot pack data into pointer");

//////////////////////////////////////////////////////////////////////////

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
    mPhysicsWorld->SetContactListener(this);

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
        UpdatePedsGravity(deltaTime);
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

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 10.0f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_PED;

    b2Fixture* b2fixture = physicsObject->mPhysicsBody->CreateFixture(&fixtureDef);
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

    int numFixtures = 0;
    // for each block create fixture
    for (int x = 0; x < MAP_DIMENSIONS; ++x)
    for (int y = 0; y < MAP_DIMENSIONS; ++y)
    {
        for (int z = 0; z < MAP_LAYERS_COUNT; ++z)
        {
            MapCoord currentMapCoord { x, y, z };
            BlockStyleData* blockData = gGameMap.GetBlock(currentMapCoord);
            debug_assert(blockData);

            if (blockData->mGroundType != eGroundType_Building)
                continue;

            // checek blox is inner
            {
                BlockStyleData* neighbourE = gGameMap.GetBlockClamp(currentMapCoord + MapCoord { 1, 0, 0 }); 
                BlockStyleData* neighbourW = gGameMap.GetBlockClamp(currentMapCoord - MapCoord { 1, 0, 0 }); 
                BlockStyleData* neighbourN = gGameMap.GetBlockClamp(currentMapCoord - MapCoord { 0, 1, 0 }); 
                BlockStyleData* neighbourS = gGameMap.GetBlockClamp(currentMapCoord + MapCoord { 0, 1, 0 });

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
            fixtureData.mY = y;

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
    return physicsObject;
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

void PhysicsManager::UpdatePedsGravity(Timespan deltaTime)
{
    for (Pedestrian* currPedestrian: gCarnageGame.mPedsManager.mActivePedsList)
    {
        // correct z coord on slopes
        glm::vec3 pedestrianPos = currPedestrian->mPhysicalBody->GetPosition();

        float zcoord = GetHeightAtPosition(pedestrianPos);
        if (zcoord != pedestrianPos.z)
        {
            currPedestrian->SetPosition(pedestrianPos.x, pedestrianPos.y, zcoord);
        }
    }
}

float PhysicsManager::GetHeightAtPosition(const glm::vec3& position) const
{
    float roundz = round(position.z);

    MapCoord mapcoord = { position.x, position.y, roundz };

    float height = mapcoord.z * 1.0f; // reset height to ground 
    for (;height > -MAP_BLOCK_LENGTH;)
    {
        BlockStyleData* blockData = gGameMap.GetBlockClamp(mapcoord);
        if (blockData->mGroundType == eGroundType_Air || blockData->mGroundType == eGroundType_Water) // fallthrough
        {
            height -= MAP_BLOCK_LENGTH;
            --mapcoord.z;
            continue;
        }
        
        // get block above
        BlockStyleData* aboveBlockData = gGameMap.GetBlockClamp(mapcoord + MapCoord { 0, 0, 1 });
        int slope = aboveBlockData->mSlopeType;
        if (slope == 0)
        {
            slope = blockData->mSlopeType;
        }

        if (slope) // compute slope height
        {
            float cx = position.x - mapcoord.x;
            float cy = position.y - mapcoord.y;
            height += GameMapHelpers::GetSlopeHeight(slope, cx, cy);
        }

        break;
    }
    return height;
}
