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
inline TUserDataClass* CastBodyData(b2Body* physicsBody)
{
    debug_assert(physicsBody);

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

PedPhysicsBody* PhysicsManager::CreatePhysicsBody(Pedestrian* object, const glm::vec3& position, cxx::angle_t rotationAngle)
{
    debug_assert(object);

    PedPhysicsBody* physicsObject = mPedsBodiesPool.create(mPhysicsWorld, object);
    physicsObject->SetPosition(position, rotationAngle);

    mPedsBodiesList.push_back(physicsObject);
    return physicsObject;
}

CarPhysicsBody* PhysicsManager::CreatePhysicsBody(Vehicle* object, const glm::vec3& position, cxx::angle_t rotationAngle)
{
    debug_assert(object);
    debug_assert(object->mCarStyle);

    CarPhysicsBody* physicsObject = mCarsBodiesPool.create(mPhysicsWorld, object);
    physicsObject->SetPosition(position, rotationAngle);

    mCarsBodiesList.push_back(physicsObject);
    return physicsObject;
}

ProjectilePhysicsBody* PhysicsManager::CreatePhysicsBody(Projectile* object, const glm::vec3& position, cxx::angle_t rotationAngle)
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

void PhysicsManager::DestroyPhysicsBody(PedPhysicsBody* object)
{
    debug_assert(object);
    cxx::erase_elements(mPedsBodiesList, object);

    mPedsBodiesPool.destroy(object);
}

void PhysicsManager::DestroyPhysicsBody(CarPhysicsBody* object)
{
    debug_assert(object);
    cxx::erase_elements(mCarsBodiesList, object);

    mCarsBodiesPool.destroy(object);
}

void PhysicsManager::DestroyPhysicsBody(ProjectilePhysicsBody* object)
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
            PedPhysicsBody* physicsComponentA = CastBodyData<PedPhysicsBody>(fixtureA->GetBody());
            PedPhysicsBody* physicsComponentB = CastBodyData<PedPhysicsBody>(fixtureB->GetBody());
            hasCollision = HasCollisionPedVsPed(contact, physicsComponentA, physicsComponentB);
        }
        // car vs car
        if (fixtureA->GetFilterData().categoryBits == PHYSICS_OBJCAT_CAR)
        {
            CarPhysicsBody* physicsComponentA = CastBodyData<CarPhysicsBody>(fixtureA->GetBody());
            CarPhysicsBody* physicsComponentB = CastBodyData<CarPhysicsBody>(fixtureB->GetBody());
            hasCollision = HasCollisionCarVsCar(contact, physicsComponentA, physicsComponentB);
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

            ProjectilePhysicsBody* projectileObject = CastBodyData<ProjectilePhysicsBody>(fixtureProjectile->GetBody());

            // projectile vs map solid block
            if (fixtureMapSolidBlock)
            {
                b2FixtureData_map fxdata = fixtureMapSolidBlock->GetUserData();
                if (projectileObject->ShouldContactWith(PHYSICS_OBJCAT_MAP_SOLID_BLOCK))
                {
                    ProcessProjectileVsMap(contact, projectileObject, fxdata.mX, fxdata.mZ);
                }
            }
            // projectile vs car
            else if (fixtureCar)
            {
                CarPhysicsBody* carPhysicsObject = CastBodyData<CarPhysicsBody>(fixtureCar->GetBody());
                if (projectileObject->ShouldContactWith(PHYSICS_OBJCAT_CAR))
                {
                    ProcessProjectileVsCar(contact, projectileObject, carPhysicsObject);
                }
            }
            // projectile vs ped
            else if (fixturePed)
            {
                PedPhysicsBody* pedPhysicsObject = CastBodyData<PedPhysicsBody>(fixturePed->GetBody());
                if (projectileObject->ShouldContactWith(PHYSICS_OBJCAT_PED))
                {
                    ProcessProjectileVsPed(contact, projectileObject, pedPhysicsObject);
                }
            }
        }
        else if (fixturePed)
        {
            PedPhysicsBody* pedPhysicsObject = CastBodyData<PedPhysicsBody>(fixturePed->GetBody());

            // ped vs map solid block
            if (fixtureMapSolidBlock)
            {
                b2FixtureData_map fxdata = fixtureMapSolidBlock->GetUserData();
     
                float height = gGameMap.GetHeightAtPosition(pedPhysicsObject->GetPosition());
                hasCollision = pedPhysicsObject->ShouldContactWith(PHYSICS_OBJCAT_MAP_SOLID_BLOCK) &&
                    HasCollisionPedVsMap(fxdata.mX, fxdata.mZ, height);
            }
            // ped vs car
            else if (fixtureCar)
            {
                CarPhysicsBody* carPhysicsObject = CastBodyData<CarPhysicsBody>(fixtureCar->GetBody());
                hasCollision = pedPhysicsObject->ShouldContactWith(PHYSICS_OBJCAT_CAR) &&
                    HasCollisionPedVsCar(contact, pedPhysicsObject, carPhysicsObject);
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
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    // check car vs car
    if (fixtureA->GetFilterData().categoryBits == 
        fixtureB->GetFilterData().categoryBits)
    {
        if (fixtureA->GetFilterData().categoryBits == PHYSICS_OBJCAT_CAR)
        {
            // todo
        }

        return;
    }

    // find impulse
    float maxImpulse = 0.0f;
    for (int icurr = 0; icurr < impulse->count; ++icurr)
    {
        maxImpulse = glm::max(maxImpulse, fabs(impulse->normalImpulses[icurr]));
    }

    if (maxImpulse < 0.01f) // impact is too small, ignore
        return;

    b2Fixture* fixtureMapSolidBlock = FilterFixture(fixtureA, fixtureB, PHYSICS_OBJCAT_MAP_SOLID_BLOCK);
    b2Fixture* fixturePed = FilterFixture(fixtureA, fixtureB, PHYSICS_OBJCAT_PED);
    b2Fixture* fixtureCar = FilterFixture(fixtureA, fixtureB, PHYSICS_OBJCAT_CAR);

    if (fixtureCar && fixturePed)
    {
        CarPhysicsBody* physicsComponentCar = CastBodyData<CarPhysicsBody>(fixtureCar->GetBody());
        PedPhysicsBody* physicsComponentPed = CastBodyData<PedPhysicsBody>(fixturePed->GetBody());
        HandleContactPedVsCar(contact, maxImpulse, physicsComponentPed, physicsComponentCar);
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
        BlockStyle* currentTile = gGameMap.GetBlockClamp(iposition.x, iposition.z, iposition.y);

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
        BlockStyle* currentTile = gGameMap.GetBlockClamp(iposition.x, iposition.z, iposition.y);

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

    BlockStyle* blockData = gGameMap.GetBlockClamp(mapx, mapy, mapLayer);
    return (blockData->mGroundType == eGroundType_Building);
}

bool PhysicsManager::HasCollisionCarVsMap(b2Contact* contact, b2Fixture* fixtureCar, int mapx, int mapy) const
{
    CarPhysicsBody* carPhysicsComponent = CastBodyData<CarPhysicsBody>(fixtureCar->GetBody());
    debug_assert(carPhysicsComponent);

    int mapLayer = (int) (Convert::MetersToMapUnits(carPhysicsComponent->mHeight) + 0.5f);

    // todo: temporary implementation

    BlockStyle* blockData = gGameMap.GetBlockClamp(mapx, mapy, mapLayer);
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
        PedPhysicsBody* pedPhysicsComponent = CastBodyData<PedPhysicsBody>(pedFixture->GetBody());
        CarPhysicsBody* carPhysicsComponent = CastBodyData<CarPhysicsBody>(carFixture->GetBody());
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
                currHit->mCarComponent = CastBodyData<CarPhysicsBody>(fixture->GetBody());

            }
            if (filterData.categoryBits == PHYSICS_OBJCAT_PED)
            {
                currHit = &mOutput.mHits[mOutput.mHitsCount++];
                currHit->mPedComponent = CastBodyData<PedPhysicsBody>(fixture->GetBody());
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
                currElement.mCarComponent = CastBodyData<CarPhysicsBody>(fixture->GetBody());
            }

            if (filterData.categoryBits == PHYSICS_OBJCAT_PED)
            {
                PhysicsQueryElement& currElement = mOutput.mElements[mOutput.mElementsCount++];
                currElement.mPedComponent = CastBodyData<PedPhysicsBody>(fixture->GetBody());
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

void PhysicsManager::HandleContactPedVsCar(b2Contact* contact, float impulse, PedPhysicsBody* ped, CarPhysicsBody* car)
{
    if (!ped->ShouldContactWith(PHYSICS_OBJCAT_CAR))
        return;

    ped->mReferencePed->ReceiveDamageFromCar(car->mReferenceCar, impulse);
}

bool PhysicsManager::ProcessProjectileVsMap(b2Contact* contact, ProjectilePhysicsBody* projectile, int mapx, int mapy) const
{
    // check same height
    int layer = (int) (Convert::MetersToMapUnits(projectile->mHeight) + 0.5f);

    BlockStyle* mapBlock = gGameMap.GetBlockClamp(mapx, mapy, layer);
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