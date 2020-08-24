#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "GameCheatsWindow.h"
#include "CarnageGame.h"
#include "Pedestrian.h"
#include "TimeManager.h"
#include "Box2D_Helpers.h"

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

// choose fixture by category bits (any of it)
inline b2Fixture* FilterFixture(b2Fixture* fixtureA, b2Fixture* fixtureB, unsigned short categoryBits)
{
    if ((fixtureA->GetFilterData().categoryBits & categoryBits) > 0)
        return fixtureA;

    if ((fixtureB->GetFilterData().categoryBits & categoryBits) > 0)
        return fixtureB;

    return nullptr;
}

template<typename TUserDataClass>
inline TUserDataClass* CastFixtureBody(b2Fixture* fixture)
{
    debug_assert(fixture);
    const b2Body* physicsBody = fixture->GetBody();

    void* userdata = physicsBody->GetUserData();
    debug_assert(userdata);
    return (TUserDataClass*) userdata;
}

//////////////////////////////////////////////////////////////////////////

PhysicsManager gPhysics;

PhysicsManager::PhysicsManager()
    : mMapCollisionShape()
    , mPhysicsWorld()
    , mGravityForce()
{
}

bool PhysicsManager::InitPhysicsWorld()
{
    b2Vec2 gravity {0.0f, 0.0f}; // default gravity shoild be disabled
    mPhysicsWorld = new b2World(gravity);
    mPhysicsWorld->SetContactListener(this);

    mSimulationStepTime = 1.0f / std::max(gSystem.mConfig.mPhysicsFramerate, 1.0f);
    mGravityForce = Convert::MapUnitsToMeters(0.5f);

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

    while (mSimulationTimeAccumulator >= mSimulationStepTime)
    {
        ProcessSimulationStep();
        mSimulationTimeAccumulator -= mSimulationStepTime;
    }
    ProcessInterpolation();
}

void PhysicsManager::ProcessSimulationStep()
{
    const int velocityIterations = 6;
    const int positionIterations = 2;

    // get previous position
    for (PhysicsBody* currComponent: mCarsBodiesList)
    {
        currComponent->mPreviousPosition = currComponent->mSmoothPosition = currComponent->GetPosition();
    }
    for (PhysicsBody* currComponent: mPedsBodiesList)
    {
        currComponent->mPreviousPosition = currComponent->mSmoothPosition = currComponent->GetPosition();
    }
    for (PhysicsBody* currComponent: mProjectileBodiesList)
    {
        currComponent->mPreviousPosition = currComponent->mSmoothPosition = currComponent->GetPosition();
    }

    mPhysicsWorld->Step(mSimulationStepTime, velocityIterations, positionIterations);

    // process physics components
    for (size_t i = 0, NumElements = mCarsBodiesList.size(); i < NumElements; ++i)
    {
        mCarsBodiesList[i]->SimulationStep();
    }
    for (size_t i = 0, NumElements = mPedsBodiesList.size(); i < NumElements; ++i)
    {
        mPedsBodiesList[i]->SimulationStep();
    }
    for (size_t i = 0, NumElements = mProjectileBodiesList.size(); i < NumElements; ++i)
    {
        mProjectileBodiesList[i]->SimulationStep();
    }

    FixedStepGravity();
}

void PhysicsManager::ProcessInterpolation()
{
    float mixFactor = mSimulationTimeAccumulator / mSimulationStepTime;

    for (PhysicsBody* currComponent: mCarsBodiesList)
    {
        currComponent->mSmoothPosition = glm::lerp(currComponent->mPreviousPosition, currComponent->GetPosition(), mixFactor);
    }

    for (PhysicsBody* currComponent: mPedsBodiesList)
    {
        currComponent->mSmoothPosition = glm::lerp(currComponent->mPreviousPosition, currComponent->GetPosition(), mixFactor);
    }

    for (PhysicsBody* currComponent: mProjectileBodiesList)
    {
        currComponent->mSmoothPosition = glm::lerp(currComponent->mPreviousPosition, currComponent->GetPosition(), mixFactor);
    }
}

PedPhysicsBody* PhysicsManager::CreatePhysicsObject(Pedestrian* object, const glm::vec3& position, cxx::angle_t rotationAngle)
{
    debug_assert(object);

    PedPhysicsBody* physicsObject = mPedsBodiesPool.create(mPhysicsWorld, object);
    physicsObject->SetPosition(position, rotationAngle);

    mPedsBodiesList.push_back(physicsObject);
    return physicsObject;
}

CarPhysicsBody* PhysicsManager::CreatePhysicsObject(Vehicle* object, const glm::vec3& position, cxx::angle_t rotationAngle)
{
    debug_assert(object);
    debug_assert(object->mCarStyle);

    CarPhysicsBody* physicsObject = mCarsBodiesPool.create(mPhysicsWorld, object);
    physicsObject->SetPosition(position, rotationAngle);

    mCarsBodiesList.push_back(physicsObject);
    return physicsObject;
}

ProjectilePhysicsBody* PhysicsManager::CreatePhysicsObject(Projectile* object, const glm::vec3& position, cxx::angle_t rotationAngle)
{
    debug_assert(object);

    ProjectilePhysicsBody* physicsObject = mProjectileBodiesPool.create(mPhysicsWorld, object);
    physicsObject->SetPosition(position, rotationAngle);

    mProjectileBodiesList.push_back(physicsObject);
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
        MapBlockInfo* blockData = gGameMap.GetBlock(x, y, layer);
        debug_assert(blockData);

        if (blockData->mGroundType != eGroundType_Building)
            continue;

        // checek blox is inner
        {
            MapBlockInfo* neighbourE = gGameMap.GetBlockClamp(x + 1, y, layer); 
            MapBlockInfo* neighbourW = gGameMap.GetBlockClamp(x - 1, y, layer); 
            MapBlockInfo* neighbourN = gGameMap.GetBlockClamp(x, y - 1, layer); 
            MapBlockInfo* neighbourS = gGameMap.GetBlockClamp(x, y + 1, layer);

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

        box2d::vec2 shapeCenter (x + 0.5f, y + 0.5f);
        shapeCenter = Convert::MapUnitsToMeters(shapeCenter);
       
        box2d::vec2 shapeLength (0.5f, 0.5f);
        shapeLength = Convert::MapUnitsToMeters(shapeLength);
        
        b2shapeDef.SetAsBox(shapeLength.x, shapeLength.y, shapeCenter, 0.0f);

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

void PhysicsManager::DestroyPhysicsObject(PedPhysicsBody* object)
{
    debug_assert(object);
    cxx::erase_elements(mPedsBodiesList, object);

    mPedsBodiesPool.destroy(object);
}

void PhysicsManager::DestroyPhysicsObject(CarPhysicsBody* object)
{
    debug_assert(object);
    cxx::erase_elements(mCarsBodiesList, object);

    mCarsBodiesPool.destroy(object);
}

void PhysicsManager::DestroyPhysicsObject(ProjectilePhysicsBody* object)
{
    debug_assert(object);
    cxx::erase_elements(mProjectileBodiesList, object);

    mProjectileBodiesPool.destroy(object);
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

    bool hasCollision = true;
    if (fixtureA->GetFilterData().categoryBits == 
        fixtureB->GetFilterData().categoryBits)
    {
        // ped vs ped
        if (fixtureA->GetFilterData().categoryBits == PHYSICS_OBJCAT_PED)
        {
            PedPhysicsBody* pedA = CastFixtureBody<PedPhysicsBody>(fixtureA);
            PedPhysicsBody* pedB = CastFixtureBody<PedPhysicsBody>(fixtureB);
            hasCollision = HasCollisionPedVsPed(contact, pedA, pedB);
        }
        // car vs car
        if (fixtureA->GetFilterData().categoryBits == PHYSICS_OBJCAT_CAR)
        {
            CarPhysicsBody* carA = CastFixtureBody<CarPhysicsBody>(fixtureA);
            CarPhysicsBody* carB = CastFixtureBody<CarPhysicsBody>(fixtureB);
            hasCollision = HasCollisionCarVsCar(contact, carA, carB);
        }
    }
    else
    {
        b2Fixture* fixtureMapSolidBlock = FilterFixture(fixtureA, fixtureB, PHYSICS_OBJCAT_MAP_SOLID_BLOCK);
        b2Fixture* fixturePed = FilterFixture(fixtureA, fixtureB, PHYSICS_OBJCAT_PED);
        b2Fixture* fixtureCar = FilterFixture(fixtureA, fixtureB, PHYSICS_OBJCAT_CAR);
        b2Fixture* fixtureProjectile = FilterFixture(fixtureA, fixtureB, PHYSICS_OBJCAT_PROJECTILE);

        if (fixtureProjectile)
        {
            hasCollision = false; // projectiles doesnt collide

            ProjectilePhysicsBody* projectile = CastFixtureBody<ProjectilePhysicsBody>(fixtureProjectile);

            // projectile vs map solid block
            if (fixtureMapSolidBlock)
            {
                b2FixtureData_map fxdata = fixtureMapSolidBlock->GetUserData();
                if (projectile->ShouldContactWith(PHYSICS_OBJCAT_MAP_SOLID_BLOCK))
                {
                    ProcessProjectileVsMap(contact, projectile, fxdata.mX, fxdata.mZ);
                }
            }
            // projectile vs car
            else if (fixtureCar)
            {
                CarPhysicsBody* car = CastFixtureBody<CarPhysicsBody>(fixtureCar);
                if (projectile->ShouldContactWith(PHYSICS_OBJCAT_CAR))
                {
                    ProcessProjectileVsCar(contact, projectile, car);
                }
            }
            // projectile vs ped
            else if (fixturePed)
            {
                PedPhysicsBody* ped = CastFixtureBody<PedPhysicsBody>(fixturePed);
                if (projectile->ShouldContactWith(PHYSICS_OBJCAT_PED))
                {
                    ProcessProjectileVsPed(contact, projectile, ped);
                }
            }
        }
        else if (fixturePed)
        {
            PedPhysicsBody* ped = CastFixtureBody<PedPhysicsBody>(fixturePed);

            // ped vs map solid block
            if (fixtureMapSolidBlock)
            {
                b2FixtureData_map fxdata = fixtureMapSolidBlock->GetUserData();
     
                float height = gGameMap.GetHeightAtPosition(ped->GetPosition());
                hasCollision = ped->ShouldContactWith(PHYSICS_OBJCAT_MAP_SOLID_BLOCK) &&
                    HasCollisionPedVsMap(fxdata.mX, fxdata.mZ, height);
            }
            // ped vs car
            else if (fixtureCar)
            {
                CarPhysicsBody* car = CastFixtureBody<CarPhysicsBody>(fixtureCar);
                hasCollision = ped->ShouldContactWith(PHYSICS_OBJCAT_CAR) &&
                    HasCollisionPedVsCar(contact, ped, car);
            }
        }
        // car vs map solid block
        else if (fixtureCar && fixtureMapSolidBlock)
        {
            b2FixtureData_map fxdata = fixtureMapSolidBlock->GetUserData();
            hasCollision = HasCollisionCarVsMap(contact, fixtureCar, fxdata.mX, fxdata.mZ);
        }
    }

    contact->SetEnabled(hasCollision);
}

void PhysicsManager::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
    if (impulse->count < 1 || contact->GetManifold()->pointCount < 1)
        return;

    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    // check car vs car
    if (fixtureA->GetFilterData().categoryBits == 
        fixtureB->GetFilterData().categoryBits)
    {
        if (fixtureA->GetFilterData().categoryBits == PHYSICS_OBJCAT_CAR)
        {
            CarPhysicsBody* carA = CastFixtureBody<CarPhysicsBody>(fixtureA);
            CarPhysicsBody* carB = CastFixtureBody<CarPhysicsBody>(fixtureB);
            HandleCollision(contact, carA, carB, impulse);
        }
    }
    else
    {
        b2Fixture* fixtureMapSolidBlock = FilterFixture(fixtureA, fixtureB, PHYSICS_OBJCAT_MAP_SOLID_BLOCK);
        b2Fixture* fixturePed = FilterFixture(fixtureA, fixtureB, PHYSICS_OBJCAT_PED);
        b2Fixture* fixtureCar = FilterFixture(fixtureA, fixtureB, PHYSICS_OBJCAT_CAR);

        if (fixtureCar && fixturePed)
        {
            CarPhysicsBody* car = CastFixtureBody<CarPhysicsBody>(fixtureCar);
            PedPhysicsBody* ped = CastFixtureBody<PedPhysicsBody>(fixturePed);
            HandleCollision(contact, ped, car, impulse);
        }
    }
}

void PhysicsManager::FixedStepGravity()
{
    // todo: cleanup this mess

    // cars
    for (size_t i = 0, NumElements = mCarsBodiesList.size(); i < NumElements; ++i)
    {

        CarPhysicsBody* physicsComponent = static_cast<CarPhysicsBody*>(mCarsBodiesList[i]);
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
        MapBlockInfo* currentTile = gGameMap.GetBlockClamp(iposition.x, iposition.z, iposition.y);

        if (currentTile->mGroundType == eGroundType_Water)
        {
            physicsComponent->HandleWaterContact();
        }
    }

    // pedestrians
    for (size_t i = 0, NumElements = mPedsBodiesList.size(); i < NumElements; ++i)
    {
        PedPhysicsBody* physicsComponent = static_cast<PedPhysicsBody*>(mPedsBodiesList[i]);
        Pedestrian* currPedestrian = physicsComponent->mReferencePed;

        if (currPedestrian->mCurrentCar)
        {
            glm::vec3 carPosition = currPedestrian->mCurrentCar->mPhysicsBody->GetPosition();

            physicsComponent->mHeight = carPosition.y;
            continue;
        }

        if (physicsComponent->mWaterContact)
            continue;

        glm::vec3 position = physicsComponent->GetPosition();

        // process fall
        float newHeight = gGameMap.GetHeightAtPosition(position, false);

        bool onTheGround = newHeight > (position.y - 0.00f);
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
            physicsComponent->mHeight -= (mSimulationStepTime * mGravityForce);
        }
        else
        {
            physicsComponent->mHeight = newHeight;
        }

        // handle water contact
        glm::ivec3 iposition = physicsComponent->GetPosition();
        MapBlockInfo* currentTile = gGameMap.GetBlockClamp(iposition.x, iposition.z, iposition.y);

        if (currentTile->mGroundType == eGroundType_Water)
        {
            physicsComponent->HandleWaterContact();
        }
    }
}

bool PhysicsManager::HasCollisionPedVsPed(b2Contact* contact, PedPhysicsBody* pedA, PedPhysicsBody* pedB) const
{
    // todo: temporary implementation

    return false;
}

bool PhysicsManager::HasCollisionCarVsCar(b2Contact* contact, CarPhysicsBody* carA, CarPhysicsBody* carB) const
{
    int carLayerA = (int) (Convert::MetersToMapUnits(carA->mHeight) + 0.5f);
    int carLayerB = (int) (Convert::MetersToMapUnits(carB->mHeight) + 0.5f);

    // todo: handle slopes

    return carLayerA == carLayerB;
}

bool PhysicsManager::HasCollisionPedVsMap(int mapx, int mapy, float height) const
{
    int mapLayer = (int) (Convert::MetersToMapUnits(height) + 0.5f);

    // todo: temporary implementation

    MapBlockInfo* blockData = gGameMap.GetBlockClamp(mapx, mapy, mapLayer);
    return (blockData->mGroundType == eGroundType_Building);
}

bool PhysicsManager::HasCollisionCarVsMap(b2Contact* contact, b2Fixture* fixtureCar, int mapx, int mapy) const
{
    CarPhysicsBody* carPhysicsComponent = CastFixtureBody<CarPhysicsBody>(fixtureCar);
    debug_assert(carPhysicsComponent);

    int mapLayer = (int) (Convert::MetersToMapUnits(carPhysicsComponent->mHeight) + 0.5f);

    // todo: temporary implementation

    MapBlockInfo* blockData = gGameMap.GetBlockClamp(mapx, mapy, mapLayer);
    return (blockData->mGroundType == eGroundType_Building);
}

bool PhysicsManager::HasCollisionPedVsCar(b2Contact* contact, PedPhysicsBody* ped, CarPhysicsBody* car) const
{
    // check car bounds height
    // todo: get car height!
    return ((ped->mHeight >= car->mHeight) && 
        (ped->mHeight <= (car->mHeight + 2.0f)));
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

    b2Fixture* pedFixture = FilterFixture(contact->GetFixtureA(), contact->GetFixtureB(), PHYSICS_OBJCAT_PED | PHYSICS_OBJCAT_PED_SENSOR);
    b2Fixture* carFixture = FilterFixture(contact->GetFixtureA(), contact->GetFixtureB(), PHYSICS_OBJCAT_CAR);
    b2Fixture* mapSolidBlockFixture = FilterFixture(contact->GetFixtureA(), contact->GetFixtureB(), PHYSICS_OBJCAT_MAP_SOLID_BLOCK);
    
    if (pedFixture && carFixture)
    {
        PedPhysicsBody* pedPhysicsComponent = CastFixtureBody<PedPhysicsBody>(pedFixture);
        CarPhysicsBody* carPhysicsComponent = CastFixtureBody<CarPhysicsBody>(carFixture);
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

void PhysicsManager::QueryObjectsLinecast(const glm::vec2& pointA, const glm::vec2& pointB, PhysicsLinecastResult& outputResult) const
{
    outputResult.Clear();

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
                currHit->mCarComponent = CastFixtureBody<CarPhysicsBody>(fixture);

            }
            if (filterData.categoryBits == PHYSICS_OBJCAT_PED)
            {
                currHit = &mOutput.mHits[mOutput.mHitsCount++];
                currHit->mPedComponent = CastFixtureBody<PedPhysicsBody>(fixture);
            }
            if (currHit)
            {
                currHit->mIntersectionPoint.x = point.x;
                currHit->mIntersectionPoint.y = point.y;
                currHit->mNormal.x = normal.x;
                currHit->mNormal.y = normal.y;
            }
            return 1.0f;
        }
    public:
        PhysicsLinecastResult& mOutput;
    };

    _raycast_callback raycast_callback {outputResult};
    box2d::vec2 p1 = pointA;
    box2d::vec2 p2 = pointB;
    mPhysicsWorld->RayCast(&raycast_callback, p1, p2);
}

void PhysicsManager::QueryObjectsWithinBox(const glm::vec2& aaboxCenter, const glm::vec2& aabboxExtents, PhysicsQueryResult& outputResult) const
{
    outputResult.Clear();

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
                currElement.mCarComponent = CastFixtureBody<CarPhysicsBody>(fixture);
            }

            if (filterData.categoryBits == PHYSICS_OBJCAT_PED)
            {
                PhysicsQueryElement& currElement = mOutput.mElements[mOutput.mElementsCount++];
                currElement.mPedComponent = CastFixtureBody<PedPhysicsBody>(fixture);
            }
            return true;
        }
    public:
        PhysicsQueryResult& mOutput;
    };
    _query_callback query_callback {outputResult};

    b2AABB aabb;
    aabb.lowerBound.x = (aaboxCenter.x - aabboxExtents.x);
    aabb.lowerBound.y = (aaboxCenter.y - aabboxExtents.y);
    aabb.upperBound.x = (aaboxCenter.x + aabboxExtents.x);
    aabb.upperBound.y = (aaboxCenter.y + aabboxExtents.y);
    mPhysicsWorld->QueryAABB(&query_callback, aabb);
}

void PhysicsManager::HandleCollision(b2Contact* contact, PedPhysicsBody* ped, CarPhysicsBody* car, const b2ContactImpulse* impulse)
{
    if (!ped->ShouldContactWith(PHYSICS_OBJCAT_CAR))
        return;

    ped->mReferencePed->ReceiveDamageFromCar(car->mReferenceCar);
}

void PhysicsManager::HandleCollision(b2Contact* contact, CarPhysicsBody* carA, CarPhysicsBody* carB, const b2ContactImpulse* impulse)
{
    int pointCount = contact->GetManifold()->pointCount;
    float maxImpulse = 0.0f;
    for (int i = 0; i < pointCount; ++i) 
    {
        maxImpulse = b2Max(maxImpulse, impulse->normalImpulses[i]);
    }

    if (maxImpulse < 62.0f) // todo: magic numbers
        return;

    b2WorldManifold wmanifold;
    contact->GetWorldManifold(&wmanifold);

    glm::vec2 contactPoint = box2d::vec2(wmanifold.points[0]);

    for (CarPhysicsBody* currCar: {carA, carB})
    {
        glm::vec3 crashPoint ( contactPoint.x, currCar->mHeight, contactPoint.y );
        currCar->mReferenceCar->ReceiveDamageFromCrash(crashPoint, maxImpulse);
    }
}

bool PhysicsManager::ProcessProjectileVsMap(b2Contact* contact, ProjectilePhysicsBody* projectile, int mapx, int mapy) const
{
    // check same height
    int layer = (int) (Convert::MetersToMapUnits(projectile->mHeight) + 0.5f);

    MapBlockInfo* mapBlock = gGameMap.GetBlockClamp(mapx, mapy, layer);
    if (mapBlock->mGroundType != eGroundType_Building)
        return false;

    // get collision point
    b2WorldManifold wmanifold;
    contact->GetWorldManifold(&wmanifold);

    int pointsCount = contact->GetManifold()->pointCount;
    if (pointsCount > 0)
    {
        glm::vec3 contactPoint( 
            wmanifold.points[0].x, projectile->mHeight, 
            wmanifold.points[0].y );

        projectile->mReferenceProjectile->SetContactDetected(contactPoint, nullptr);
        return true;
    }
    return false;
}

bool PhysicsManager::ProcessProjectileVsCar(b2Contact* contact, ProjectilePhysicsBody* projectile, CarPhysicsBody* car) const
{
    // check car bounds height
    // todo: get car height!
    bool hasContact = ((projectile->mHeight >= car->mHeight) && 
        (projectile->mHeight <= (car->mHeight + 2.0f)));

    if (!hasContact)
        return false;

    // get collision point
    b2WorldManifold wmanifold;
    contact->GetWorldManifold(&wmanifold);

    int pointsCount = contact->GetManifold()->pointCount;
    if (pointsCount > 0)
    {
        glm::vec3 contactPoint( 
            wmanifold.points[0].x, projectile->mHeight, 
            wmanifold.points[0].y );

        projectile->mReferenceProjectile->SetContactDetected(contactPoint, car->mReferenceCar);
        return true;
    }
    return false;
}

bool PhysicsManager::ProcessProjectileVsPed(b2Contact* contact, ProjectilePhysicsBody* projectile, PedPhysicsBody* ped) const
{
    // check ped bounds height
    // todo: get car height!
    bool hasContact = ((projectile->mHeight >= ped->mHeight) && 
        (projectile->mHeight <= (ped->mHeight + 2.0f)));

    if (!hasContact)
        return false;

    // get collision point
    b2WorldManifold wmanifold;
    contact->GetWorldManifold(&wmanifold);

    int pointsCount = contact->GetManifold()->pointCount;
    if (pointsCount > 0)
    {
        glm::vec3 contactPoint( 
            wmanifold.points[0].x, projectile->mHeight, 
            wmanifold.points[0].y );

        projectile->mReferenceProjectile->SetContactDetected(contactPoint, ped->mReferencePed);
        return true;
    }
    return false;
}