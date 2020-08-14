#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "GameCheatsWindow.h"
#include "CarnageGame.h"
#include "Pedestrian.h"
#include "TimeManager.h"

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

bool PhysicsManager::InitPhysicsWorld()
{
    b2Vec2 gravity {0.0f, 0.0f}; // default gravity shoild be disabled
    mPhysicsWorld = new b2World(gravity);
    mPhysicsWorld->SetContactListener(this);

    double physicsFramerate = gSystem.mConfig.mPhysicsFramerate;
    debug_assert(physicsFramerate > 0.0);
    mSimulationStepTime = (float) (1.0 / physicsFramerate);
    debug_assert(mSimulationStepTime > 0.0);

    CreateMapCollisionShape();
    return true;
}

void PhysicsManager::FreePhysicsWorld()
{
    if (mMapCollisionShape)
    {
        mPhysicsWorld->DestroyBody(mMapCollisionShape);
        mMapCollisionShape = nullptr;
    }
    SafeDelete(mPhysicsWorld);
}

void PhysicsManager::UpdateFrame()
{
    mSimulationTimeAccumulator += gTimeManager.mGameFrameDelta;

    const int MaxSimulationStepsPerFrame = 5;
    int numSimulations = static_cast<int>(mSimulationTimeAccumulator / mSimulationStepTime);
    if (numSimulations > 0)
    {
        mSimulationTimeAccumulator -= (numSimulations * mSimulationStepTime);
        numSimulations = glm::min(numSimulations, MaxSimulationStepsPerFrame);
    }
    debug_assert(numSimulations <= MaxSimulationStepsPerFrame);
    debug_assert(mSimulationTimeAccumulator < mSimulationStepTime && mSimulationTimeAccumulator > -0.01f);

    for (int icurrStep = 0; icurrStep < numSimulations; ++icurrStep)
    {
        ProcessSimulationStep(icurrStep == (numSimulations - 1));
    }

    ProcessInterpolation();
}

void PhysicsManager::ProcessSimulationStep(bool resetPreviousState)
{
    const int velocityIterations = 4;
    const int positionIterations = 4;

    if (resetPreviousState)
    {
        for (CarPhysicsComponent* currComponent: mCarsBodiesList)
        {
            currComponent->mPreviousPosition = currComponent->mSmoothPosition = currComponent->GetPosition();
        }

        for (PedPhysicsComponent* currComponent: mPedsBodiesList)
        {
            currComponent->mPreviousPosition = currComponent->mSmoothPosition = currComponent->GetPosition();
        }  
    }

    mPhysicsWorld->Step(mSimulationStepTime, velocityIterations, positionIterations);

    // process cars physics components
    for (CarPhysicsComponent* currComponent: mCarsBodiesList)
    {
        currComponent->SimulationStep();
    }

    // process peds physics components
    for (PedPhysicsComponent* currComponent: mPedsBodiesList)
    {
        currComponent->SimulationStep();
    }

    FixedStepGravity();
}

void PhysicsManager::ProcessInterpolation()
{
    float mixFactor = mSimulationTimeAccumulator / mSimulationStepTime;

    for (CarPhysicsComponent* currComponent: mCarsBodiesList)
    {
        currComponent->mSmoothPosition = glm::lerp(currComponent->mPreviousPosition, currComponent->GetPosition(), mixFactor);
    }

    for (PedPhysicsComponent* currComponent: mPedsBodiesList)
    {
        currComponent->mSmoothPosition = glm::lerp(currComponent->mPreviousPosition, currComponent->GetPosition(), mixFactor);
    }
}

PedPhysicsComponent* PhysicsManager::CreatePhysicsComponent(Pedestrian* pedestrian, const glm::vec3& position, cxx::angle_t rotationAngle)
{
    debug_assert(pedestrian);

    PedPhysicsComponent* physicsObject = mPedsBodiesPool.create(mPhysicsWorld, position, rotationAngle);
    physicsObject->mReferencePed = pedestrian;

    mPedsBodiesList.insert(&physicsObject->mPhysicsComponentsListNode);
    return physicsObject;
}

CarPhysicsComponent* PhysicsManager::CreatePhysicsComponent(Vehicle* car, const glm::vec3& position, cxx::angle_t rotationAngle, CarStyle* desc)
{
    debug_assert(desc);
    debug_assert(car);

    CarPhysicsComponent* physicsObject = mCarsBodiesPool.create(mPhysicsWorld, desc, position, rotationAngle);
    physicsObject->mReferenceCar = car;

    mCarsBodiesList.insert(&physicsObject->mPhysicsComponentsListNode);
    return physicsObject;
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
            (x + 0.5f) * PHYSICS_SCALE, 
            (y + 0.5f) * PHYSICS_SCALE
        };
        b2shapeDef.SetAsBox(0.5f * PHYSICS_SCALE, 0.5f * PHYSICS_SCALE, center, 0.0f);

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
    if (mPedsBodiesList.contains(&object->mPhysicsComponentsListNode))
    {
        mPedsBodiesList.remove(&object->mPhysicsComponentsListNode);
    }
    else
    {
        debug_assert(false);
    }
    mPedsBodiesPool.destroy(object);
}

void PhysicsManager::DestroyPhysicsComponent(CarPhysicsComponent* object)
{
    debug_assert(object);
    if (mCarsBodiesList.contains(&object->mPhysicsComponentsListNode))
    {
        mCarsBodiesList.remove(&object->mPhysicsComponentsListNode);
    }
    else
    {
        debug_assert(false);
    }
    mCarsBodiesPool.destroy(object);
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
        if (fixtureA->GetFilterData().categoryBits == PHYSICS_OBJCAT_PED &&
            fixtureB->GetFilterData().categoryBits == PHYSICS_OBJCAT_PED)
        {
            PedPhysicsComponent* physicsComponentA = (PedPhysicsComponent*) fixtureA->GetBody()->GetUserData();
            PedPhysicsComponent* physicsComponentB = (PedPhysicsComponent*) fixtureB->GetBody()->GetUserData();
            hasCollision = CollidePedVsPed(contact, physicsComponentA, physicsComponentB);
        }

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

        if (hasCollision && fixtureMapSolidBlock && fixtureCar)
        {
            b2FixtureData_map fxdata = fixtureMapSolidBlock->GetUserData();
            hasCollision = HasCollisionCarVsMap(contact, fixtureCar, fxdata.mX, fxdata.mZ);
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

void PhysicsManager::FixedStepGravity()
{
    // cars
    for (Vehicle* currCar: gGameObjectsManager.mCarsList)
    {
        CarPhysicsComponent* physicsComponent = currCar->mPhysicsComponent;

        if (physicsComponent->mWaterContact)
            continue;

        glm::vec3 position = physicsComponent->GetPosition();

        // process falling
        float newHeight = gGameMap.GetHeightAtPosition(position, false);

        bool onTheGround = newHeight > (position.y - 0.01f);
        if (!onTheGround)
        {
            physicsComponent->mHeight -= (mSimulationStepTime * 0.5f);
        }
        else
        {
            physicsComponent->mHeight = newHeight;
        }

        // handle water contact
        glm::ivec3 iposition = physicsComponent->GetPosition();
        BlockStyle* currentTile = gGameMap.GetBlockClamp(iposition.x, iposition.z, iposition.y);

        if (currentTile->mGroundType == eGroundType_Water)
        {
            physicsComponent->HandleWaterContact();
        }
    }

    // pedestrians
    for (Pedestrian* currPedestrian: gGameObjectsManager.mPedestriansList)
    {
        PedPhysicsComponent* physicsComponent = currPedestrian->mPhysicsComponent;
        if (currPedestrian->mCurrentCar)
        {
            glm::vec3 carPosition = currPedestrian->mCurrentCar->mPhysicsComponent->GetPosition();

            physicsComponent->mHeight = carPosition.y;
            continue;
        }

        if (physicsComponent->mWaterContact)
            continue;

        glm::vec3 position = physicsComponent->GetPosition();

        // process fall
        float newHeight = gGameMap.GetHeightAtPosition(position, false);

        bool onTheGround = newHeight > (position.y - 0.01f);
        if (physicsComponent->mFalling)
        {
            if (onTheGround)
            {
                physicsComponent->HandleFallEnd();
            }
        }
        else
        {
            float distanceToGround = position.y - newHeight;
            if (distanceToGround > 0.99f)
            {
                physicsComponent->HandleFallBegin(distanceToGround);
            }
        }

        if (!onTheGround && physicsComponent->mFalling)
        {
            physicsComponent->mHeight -= (mSimulationStepTime * 0.5f);
        }
        else
        {
            physicsComponent->mHeight = newHeight;
        }

        // handle water contact
        glm::ivec3 iposition = physicsComponent->GetPosition();
        BlockStyle* currentTile = gGameMap.GetBlockClamp(iposition.x, iposition.z, iposition.y);

        if (currentTile->mGroundType == eGroundType_Water)
        {
            physicsComponent->HandleWaterContact();
        }
    }
}

bool PhysicsManager::CollidePedVsPed(b2Contact* contact, PedPhysicsComponent* pedA, PedPhysicsComponent* pedB)
{
    // todo: temporary implementation

    return false;
}

bool PhysicsManager::HasCollisionPedestrianVsMap(int mapx, int mapz, float height) const
{
    int map_layer = (int) (height + 0.5f);

    // todo: temporary implementation

    BlockStyle* blockData = gGameMap.GetBlockClamp(mapx, mapz, map_layer);
    return (blockData->mGroundType == eGroundType_Building);
}

bool PhysicsManager::HasCollisionCarVsMap(b2Contact* contact, b2Fixture* fixtureCar, int mapx, int mapz) const
{
    CarPhysicsComponent* carPhysicsComponent = (CarPhysicsComponent*) fixtureCar->GetBody()->GetUserData();
    debug_assert(carPhysicsComponent);
    int map_layer = (int) (carPhysicsComponent->mHeight + 0.5f);

    // todo: temporary implementation

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

void PhysicsManager::QueryObjectsLinecast(const glm::vec2& pointA, const glm::vec2& pointB, PhysicsLinecastResult& outputResult) const
{
    outputResult.SetNull();

    struct _raycast_callback: public b2RayCastCallback
    {
    public:
        _raycast_callback(PhysicsLinecastResult& out)
            : mOutput(out)
        {
        }
	    float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) override
        {
            if (mOutput.IsFull())
                return 0.0f;

            PhysicsLinecastHit* currHit = nullptr;

            const b2Filter& filterData = fixture->GetFilterData();
            if (filterData.categoryBits == PHYSICS_OBJCAT_CAR)
            {
                currHit = &mOutput.mHits[mOutput.mHitsCount++];
                currHit->mCarComponent = (CarPhysicsComponent*) fixture->GetBody()->GetUserData();

            }
            if (filterData.categoryBits == PHYSICS_OBJCAT_PED)
            {
                currHit = &mOutput.mHits[mOutput.mHitsCount++];
                currHit->mPedComponent = (PedPhysicsComponent*) fixture->GetBody()->GetUserData();
            }
            if (currHit)
            {
                currHit->mIntersectionPoint.x = (point.x / PHYSICS_SCALE);
                currHit->mIntersectionPoint.y = (point.y / PHYSICS_SCALE);
                currHit->mNormal.x = normal.x;
                currHit->mNormal.y = normal.y;
            }
            return 1.0f;
        }
    public:
        PhysicsLinecastResult& mOutput;
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
            if (mOutput.IsFull())
                return false;

            const b2Filter& filterData = fixture->GetFilterData();
            if (filterData.categoryBits == PHYSICS_OBJCAT_CAR)
            {
                PhysicsQueryElement& currElement = mOutput.mElements[mOutput.mElementsCount++];
                currElement.mCarComponent = (CarPhysicsComponent*) fixture->GetBody()->GetUserData();
            }

            if (filterData.categoryBits == PHYSICS_OBJCAT_PED)
            {
                PhysicsQueryElement& currElement = mOutput.mElements[mOutput.mElementsCount++];
                currElement.mPedComponent = (PedPhysicsComponent*) fixture->GetBody()->GetUserData();
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