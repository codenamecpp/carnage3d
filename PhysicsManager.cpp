#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "GameCheatsWindow.h"
#include "CarnageGame.h"

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
}

bool PhysicsManager::Initialize()
{
    b2Vec2 gravity {0.0f, 0.0f}; // default gravity shoild be disabled
    mPhysicsWorld = new b2World(gravity);
    mPhysicsWorld->SetContactListener(this);
    //mPhysicsWorld->SetAutoClearForces(true);

    // create collsition body for map
    CreateMapCollisionShape();
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
        FixedStepPedsGravity();
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
    bodyDef.position = {position.x * PHYSICS_SCALE, position.z * PHYSICS_SCALE};
    bodyDef.angle = glm::radians(angleDegrees);
    bodyDef.fixedRotation = true;
    bodyDef.userData = physicsObject;

    physicsObject->mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(physicsObject->mPhysicsBody);
    physicsObject->mDepth = 0.5f;
    physicsObject->mHeight = position.y;
    
    b2CircleShape shapeDef;
    shapeDef.m_radius = PHYSICS_PED_BOUNDING_SPHERE_RADIUS * PHYSICS_SCALE;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 0.3f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_PED;

    b2Fixture* b2fixture = physicsObject->mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);

    return physicsObject;
}

PhysicsObject* PhysicsManager::CreateVehicleBody(const glm::vec3& position, float angleDegrees, CarStyle* desc)
{
    debug_assert(desc);

    PhysicsObject* physicsObject = mObjectsPool.create();
    physicsObject->mPhysicsWorld = mPhysicsWorld;

    // create body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {position.x * PHYSICS_SCALE, position.z * PHYSICS_SCALE};
    bodyDef.angle = glm::radians(angleDegrees);
    bodyDef.userData = physicsObject;
    bodyDef.linearDamping = 0.15f;
    bodyDef.angularDamping = 0.3f;

    physicsObject->mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(physicsObject->mPhysicsBody);
    physicsObject->mDepth = (1.0f * desc->mDepth) / MAP_BLOCK_TEXTURE_DIMS;
    physicsObject->mHeight = position.y;
    
    b2PolygonShape shapeDef;
    shapeDef.SetAsBox(((1.0f * desc->mHeight) / MAP_BLOCK_TEXTURE_DIMS) * 0.5f * PHYSICS_SCALE, 
        ((1.0f * desc->mWidth) / MAP_BLOCK_TEXTURE_DIMS) * 0.5f * PHYSICS_SCALE);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 700.0f;
    fixtureDef.friction = 0.1f;
    fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_CAR;

    b2Fixture* b2fixture = physicsObject->mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);

    return physicsObject;
}

void PhysicsManager::CreateMapCollisionShape()
{
    return;
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
            BlockStyle* blockData = gGameMap.GetBlock(x, y, layer);
            debug_assert(blockData);

            if (blockData->mGroundType != eGroundType_Building)
                continue;

            // checek blox is inner
            {
                BlockStyle* neighbourE = gGameMap.GetBlockClamp(x + 1, y, layer); 
                BlockStyle* neighbourW = gGameMap.GetBlockClamp(x - 1, y, layer); 
                BlockStyle* neighbourN = gGameMap.GetBlockClamp(x, y - 1, layer); 
                BlockStyle* neighbourS = gGameMap.GetBlockClamp(x, y + 1, layer);

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
                ((x * MAP_BLOCK_LENGTH) + (MAP_BLOCK_LENGTH * 0.5f)) * PHYSICS_SCALE, 
                ((y * MAP_BLOCK_LENGTH) + (MAP_BLOCK_LENGTH * 0.5f)) * PHYSICS_SCALE
            };
            b2shapeDef.SetAsBox(MAP_BLOCK_LENGTH * 0.5f * PHYSICS_SCALE, MAP_BLOCK_LENGTH * 0.5f * PHYSICS_SCALE, center, 0.0f);

            b2FixtureData_map fixtureData;
            fixtureData.mX = x;
            fixtureData.mZ = y;

            b2FixtureDef b2fixtureDef;
            b2fixtureDef.density = 0.0f;
            b2fixtureDef.shape = &b2shapeDef;
            b2fixtureDef.userData = fixtureData.mAsPointer;
            b2fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_MAP_SOLID_BLOCK;

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
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    b2Fixture* fixtureMapSolidBlock = nullptr;
    b2Fixture* fixturePed = nullptr;
    b2Fixture* fixtureCar = nullptr;
    if (fixtureA->GetFilterData().categoryBits == PHYSICS_OBJCAT_MAP_SOLID_BLOCK)
    {
        fixtureMapSolidBlock = fixtureA;
    }
    if (fixtureB->GetFilterData().categoryBits == PHYSICS_OBJCAT_MAP_SOLID_BLOCK)
    {
        fixtureMapSolidBlock = fixtureB;
    }

    if (fixtureA->GetFilterData().categoryBits == PHYSICS_OBJCAT_PED)
    {
        fixturePed = fixtureA;
    }
    if (fixtureB->GetFilterData().categoryBits == PHYSICS_OBJCAT_PED)
    {
        fixturePed = fixtureB;
    }

    if (fixtureA->GetFilterData().categoryBits == PHYSICS_OBJCAT_CAR)
    {
        fixtureCar = fixtureA;
    }
    if (fixtureB->GetFilterData().categoryBits == PHYSICS_OBJCAT_CAR)
    {
        fixtureCar = fixtureB;
    }

    bool hasCollision = true;

    if (fixtureMapSolidBlock && fixturePed)
    {
        b2FixtureData_map fxdata = fixtureMapSolidBlock->GetUserData();
        PhysicsObject* physicsObject = (PhysicsObject*) fixturePed->GetBody()->GetUserData();
        debug_assert(physicsObject);
        // detect height
        float height = gGameMap.GetHeightAtPosition(physicsObject->GetPosition());
        hasCollision = HasCollisionPedestrianVsMap(fxdata.mX, fxdata.mZ, height);
    }

    if (fixtureCar && fixturePed)
    {
        hasCollision = HasCollisionPedestrianVsCar(contact, fixturePed, fixtureCar);
    }

    contact->SetEnabled(hasCollision);
}

void PhysicsManager::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
}

void PhysicsManager::FixedStepPedsGravity()
{
    for (Pedestrian* currPedestrian: gCarnageGame.mPedsManager.mActivePedestriansList)
    {
        glm::vec3 pedestrianPos = currPedestrian->mPhysicalBody->GetPosition();

        // process falling
        float newHeight = gGameMap.GetHeightAtPosition(pedestrianPos);
        if (currPedestrian->IsFalling())
        {
            if (abs(newHeight - pedestrianPos.y) < 0.1f)
            {
                currPedestrian->mPhysicalBody->StopFalling();
            }
        }
        else
        {
            if ((pedestrianPos.y - newHeight) >= MAP_BLOCK_LENGTH)
            {
                currPedestrian->mPhysicalBody->StartFalling();
            }
        }

        if (currPedestrian->IsFalling())
        {
            pedestrianPos.y -= (PHYSICS_SIMULATION_STEP / 2.0f);
        }
        else
        {
            pedestrianPos.y = newHeight;
        }

        currPedestrian->SetPosition(pedestrianPos);
    }
}

bool PhysicsManager::HasCollisionPedestrianVsMap(int mapx, int mapz, float height) const
{
    int map_layer = (int) (height + 0.5f);

    // todo:
    BlockStyle* blockData = gGameMap.GetBlockClamp(mapx, mapz, map_layer);
    return (blockData->mGroundType == eGroundType_Building);
}

bool PhysicsManager::HasCollisionPedestrianVsCar(b2Contact* contact, b2Fixture* pedestrianFixture, b2Fixture* carFixture)
{
    PhysicsObject* carPhysicsObject = (PhysicsObject*) carFixture->GetBody()->GetUserData();
    PhysicsObject* pedPhysicsObject = (PhysicsObject*) pedestrianFixture->GetBody()->GetUserData();

    if (pedPhysicsObject->mSlideOverCars)
        return false;

    return true;
}
