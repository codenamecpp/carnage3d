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

    const int velocityIterations = 3;
    const int positionIterations = 2;

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

PedPhysicsComponent* PhysicsManager::CreatePhysicsComponent(Pedestrian* pedestrian, const glm::vec3& position, cxx::angle_t rotationAngle)
{
    debug_assert(pedestrian);

    PedPhysicsComponent* physicsObject = mPedsBodiesPool.create(mPhysicsWorld);
    physicsObject->mReferencePed = pedestrian;
    physicsObject->SetPosition(position, rotationAngle);
    return physicsObject;
}

CarPhysicsComponent* PhysicsManager::CreatePhysicsComponent(Vehicle* car, const glm::vec3& position, cxx::angle_t rotationAngle, CarStyle* desc)
{
    debug_assert(desc);
    debug_assert(car);

    CarPhysicsComponent* physicsObject = mCarsBodiesPool.create(mPhysicsWorld, desc);
    physicsObject->mReferenceCar = car;
    physicsObject->SetPosition(position, rotationAngle);
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
                return gtype == eGroundType_Field || gtype == eGroundType_Pawement || gtype == eGroundType_Road;
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
    if (ProcessSensorContact(contact, true))
        return;
}

void PhysicsManager::EndContact(b2Contact* contact)
{
    if (ProcessSensorContact(contact, false))
        return;
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
    for (;;)
    {
        if (hasCollision && fixturePed)
        {
            PedPhysicsComponent* physicsComponent = (PedPhysicsComponent*) fixturePed->GetBody()->GetUserData();
            debug_assert(physicsComponent);

            hasCollision = physicsComponent->ShouldCollideWith((fixtureA != fixturePed ? fixtureA : fixtureB)->GetFilterData().categoryBits);
        }
 
        if (hasCollision && fixtureMapSolidBlock && fixturePed)
        {
            b2FixtureData_map fxdata = fixtureMapSolidBlock->GetUserData();
            PhysicsComponent* physicsObject = (PhysicsComponent*) fixturePed->GetBody()->GetUserData();
            debug_assert(physicsObject);
            // detect height
            float height = gGameMap.GetHeightAtPosition(physicsObject->GetPosition());
            hasCollision = HasCollisionPedestrianVsMap(fxdata.mX, fxdata.mZ, height);
        }

        if (hasCollision && fixtureCar && fixturePed)
        {
            hasCollision = HasCollisionPedestrianVsCar(contact, fixturePed, fixtureCar);
        }

        break;
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
        PedPhysicsComponent* pedestrianBody = currPedestrian->mPhysicsComponent;
        glm::vec3 pedestrianPos = pedestrianBody->GetPosition();

        // process falling
        float newHeight = gGameMap.GetHeightAtPosition(pedestrianPos);

        pedestrianBody->mOnTheGround = newHeight > pedestrianPos.y - 0.1f;
        if (pedestrianBody->mFalling)
        {
            if (pedestrianBody->mOnTheGround)
            {
                pedestrianBody->SetFalling(false);
            }
        }
        else
        {
            if ((pedestrianPos.y - newHeight) >= MAP_BLOCK_LENGTH)
            {
                pedestrianBody->SetFalling(true);
            }
        }

        if (!pedestrianBody->mOnTheGround)
        {
            pedestrianPos.y -= (PHYSICS_SIMULATION_STEP / 2.0f);
        }
        else
        {
            pedestrianPos.y = newHeight;
        }

        pedestrianBody->SetPosition(pedestrianPos);
    }
}

bool PhysicsManager::HasCollisionPedestrianVsMap(int mapx, int mapz, float height) const
{
    int map_layer = (int) (height + 0.5f);

    // todo:
    BlockStyle* blockData = gGameMap.GetBlockClamp(mapx, mapz, map_layer);
    return (blockData->mGroundType == eGroundType_Building);
}

bool PhysicsManager::HasCollisionPedestrianVsCar(b2Contact* contact, b2Fixture* fixturePed, b2Fixture* fixtureCar)
{
    CarPhysicsComponent* carPhysicsObject = (CarPhysicsComponent*) fixtureCar->GetBody()->GetUserData();
    PedPhysicsComponent* pedPhysicsObject = (PedPhysicsComponent*) fixturePed->GetBody()->GetUserData();
    return true;
}

bool PhysicsManager::ProcessSensorContact(b2Contact* contact, bool onBegin)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
  
    // make sure only one of the fixtures was a sensor
    bool sensorA = fixtureA->IsSensor();
    bool sensorB = fixtureB->IsSensor();
    if (!(sensorA ^ sensorB))
        return false;

    PedPhysicsComponent* pedPhysicsComponent = nullptr;
    CarPhysicsComponent* carPhysicsComponent = nullptr;
    if (GetContactComponents(contact, pedPhysicsComponent, carPhysicsComponent))
    {
        if (onBegin)
        {
            pedPhysicsComponent->HandleCarContactBegin();
        }
        else
        {
            pedPhysicsComponent->HandleCarContactEnd();
        }
        return true;
    }

    return false;
}

bool PhysicsManager::GetContactComponents(b2Contact* contact, PedPhysicsComponent*& pedPhysicsObject, CarPhysicsComponent*& carPhysicsObject) const
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    b2Fixture* fixturePed = nullptr;
    b2Fixture* fixtureCar = nullptr;

    const b2Filter& filterA = fixtureA->GetFilterData();
    if ((filterA.categoryBits == PHYSICS_OBJCAT_PED) || (filterA.categoryBits == PHYSICS_OBJCAT_PED_SENSOR))
    {
        fixturePed = fixtureA;
    }
    else if (filterA.categoryBits == PHYSICS_OBJCAT_CAR)
    {
        fixtureCar = fixtureA;
    }

    const b2Filter& filterB = fixtureB->GetFilterData();
    if ((filterB.categoryBits == PHYSICS_OBJCAT_PED) || (filterB.categoryBits == PHYSICS_OBJCAT_PED_SENSOR))
    {
        fixturePed = fixtureB;
    }
    else if (filterB.categoryBits == PHYSICS_OBJCAT_CAR)
    {
        fixtureCar = fixtureB;
    }

    if (fixturePed == nullptr || fixtureCar == nullptr)
        return false;

    carPhysicsObject = (CarPhysicsComponent*) fixtureCar->GetBody()->GetUserData();
    pedPhysicsObject = (PedPhysicsComponent*) fixturePed->GetBody()->GetUserData();

    debug_assert(carPhysicsObject && pedPhysicsObject);
    return true;
}

void PhysicsManager::QueryObjects(const glm::vec2& pointA, const glm::vec2& pointB, PhysicsQueryResult& outputResult) const
{
    outputResult.SetNull();

    struct _raycast_callback: public b2RayCastCallback
    {
    public:
        _raycast_callback(PhysicsQueryResult& out)
            : mOutput(out)
        {
        }
	    float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) override
        {
            const b2Filter& filterData = fixture->GetFilterData();
            if (filterData.categoryBits == PHYSICS_OBJCAT_CAR)
            {
                CarPhysicsComponent* component = (CarPhysicsComponent*) fixture->GetBody()->GetUserData();
                if (!mOutput.AddElement(component))
                    return 0.0f;
                
            }
            if (filterData.categoryBits == PHYSICS_OBJCAT_PED)
            {
                PedPhysicsComponent* component = (PedPhysicsComponent*) fixture->GetBody()->GetUserData();
                if (!mOutput.AddElement(component))
                    return 0.0f;
            }
            return 1.0f;
        }
    public:
        PhysicsQueryResult& mOutput;
    };

    _raycast_callback raycast_callback {outputResult};
    b2Vec2 p1 { pointA.x * PHYSICS_SCALE, pointA.y * PHYSICS_SCALE };
    b2Vec2 p2 { pointB.x * PHYSICS_SCALE, pointB.y * PHYSICS_SCALE };
    mPhysicsWorld->RayCast(&raycast_callback, p1, p2);
}

void PhysicsManager::QueryObjectsWithinBox(const glm::vec2& aaboxCenter, const glm::vec2& aabboxExtents, PhysicsQueryResult& outputResult) const
{
    outputResult.SetNull();

    struct _query_callback: public b2QueryCallback
    {
    public:
        _query_callback(PhysicsQueryResult& out)
            : mOutput(out) 
        {
        }
        bool ReportFixture(b2Fixture* fixture) override
        {
            const b2Filter& filterData = fixture->GetFilterData();
            if (filterData.categoryBits == PHYSICS_OBJCAT_CAR)
            {
                CarPhysicsComponent* component = (CarPhysicsComponent*) fixture->GetBody()->GetUserData();
                if (!mOutput.AddElement(component))
                    return false;
                
            }
            if (filterData.categoryBits == PHYSICS_OBJCAT_PED)
            {
                PedPhysicsComponent* component = (PedPhysicsComponent*) fixture->GetBody()->GetUserData();
                if (!mOutput.AddElement(component))
                    return false;
            }
            return true;
        }
    public:
        PhysicsQueryResult& mOutput;
    };
    _query_callback query_callback {outputResult};

    b2AABB aabb;
    aabb.lowerBound.x = (aaboxCenter.x - aabboxExtents.x) * PHYSICS_SCALE;
    aabb.lowerBound.y = (aaboxCenter.y - aabboxExtents.y) * PHYSICS_SCALE;
    aabb.upperBound.x = (aaboxCenter.x + aabboxExtents.x) * PHYSICS_SCALE;
    aabb.upperBound.y = (aaboxCenter.y + aabboxExtents.y) * PHYSICS_SCALE;
    mPhysicsWorld->QueryAABB(&query_callback, aabb);
}