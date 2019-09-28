#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "GameCheatsWindow.h"
#include "CarnageGame.h"
#include "Pedestrian.h"

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
    : mMapCollisionShape()
    , mPhysicsWorld()
{
}

bool PhysicsManager::Initialize()
{
    b2Vec2 gravity {0.0f, 0.0f}; // default gravity shoild be disabled
    mPhysicsWorld = new b2World(gravity);
    mPhysicsWorld->SetContactListener(this);
    //mPhysicsWorld->SetAutoClearForces(true);

    CreateMapCollisionShape();
    return true;
}

void PhysicsManager::Deinit()
{
    if (mMapCollisionShape)
    {
        mPhysicsWorld->DestroyBody(mMapCollisionShape);
        mMapCollisionShape = nullptr;
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

PedPhysicsComponent* PhysicsManager::CreatePedPhysicsComponent(const glm::vec3& position, float angleDegrees)
{
    PedPhysicsComponent* physicsObject = mPedsBodiesPool.create(mPhysicsWorld);
    physicsObject->SetPosition(position, angleDegrees);
    return physicsObject;
}

CarPhysicsComponent* PhysicsManager::CreateCarPhysicsComponent(const glm::vec3& position, float angleDegrees, CarStyle* desc)
{
    debug_assert(desc);

    CarPhysicsComponent* physicsObject = mCarsBodiesPool.create(mPhysicsWorld, desc);
    physicsObject->SetPosition(position, angleDegrees);
    return physicsObject;
}

WheelPhysicsComponent* PhysicsManager::CreateWheelPhysicsComponent()
{
    debug_assert(false); // todo
    return nullptr;
}

void PhysicsManager::CreateMapCollisionShape()
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;

    mMapCollisionShape = mPhysicsWorld->CreateBody(&bodyDef);

    int numFixtures = 0;
    // for each block create fixture
    for (int x = 0; x < MAP_DIMENSIONS; ++x)
    for (int y = 0; y < MAP_DIMENSIONS; ++y)
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

        b2Fixture* b2fixture = mMapCollisionShape->CreateFixture(&b2fixtureDef);
        debug_assert(b2fixture);

        ++numFixtures;
        break; // single fixture per block column
    }
}

void PhysicsManager::DestroyPhysicsComponent(PedPhysicsComponent* object)
{
    debug_assert(object);
    mPedsBodiesPool.destroy(object);
}

void PhysicsManager::DestroyPhysicsComponent(CarPhysicsComponent* object)
{
    debug_assert(object);
    mCarsBodiesPool.destroy(object);
}

void PhysicsManager::DestroyPhysicsComponent(WheelPhysicsComponent* object)
{
    debug_assert(object);
    mWheelsBodiesPool.destroy(object);
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
        PhysicsComponent* physicsObject = (PhysicsComponent*) fixturePed->GetBody()->GetUserData();
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
    for (Pedestrian* currPedestrian: gCarnageGame.mObjectsManager.mActivePedestriansList)
    {
        glm::vec3 pedestrianPos = currPedestrian->mPhysicsComponent->GetPosition();

        // process falling
        float newHeight = gGameMap.GetHeightAtPosition(pedestrianPos);
        if (currPedestrian->IsFalling())
        {
            if (abs(newHeight - pedestrianPos.y) < 0.1f)
            {
                currPedestrian->mPhysicsComponent->mOnTheGround = true;
            }
        }
        else
        {
            if ((pedestrianPos.y - newHeight) >= MAP_BLOCK_LENGTH)
            {
                currPedestrian->mPhysicsComponent->mOnTheGround = false;
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
    PhysicsComponent* carPhysicsObject = (PhysicsComponent*) carFixture->GetBody()->GetUserData();
    PhysicsComponent* pedPhysicsObject = (PhysicsComponent*) pedestrianFixture->GetBody()->GetUserData();

    return true;
}
