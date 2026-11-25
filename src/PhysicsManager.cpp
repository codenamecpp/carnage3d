#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "GameCheatsWindow.h"
#include "CarnageGame.h"
#include "Pedestrian.h"
#include "TimeManager.h"
#include "Box2D_Helpers.h"
#include "cvars.h"
#include "ParticleEffectsManager.h"
#include "Collider.h"
#include "PhysicsBody.h"
#include "Collision.h"
#include "GameObjectHelpers.h"
#include "AudioManager.h"

//////////////////////////////////////////////////////////////////////////

static cxx::object_pool<PhysicsBody> gPhysicsBodiesPool;

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
        // walls only
        unsigned char mLayer;
        unsigned char mFace;
    };

    void* mAsPointer;
};

static_assert(sizeof(b2FixtureData_map) <= sizeof(void*), "Cannot pack data into pointer");

// Whether object can go through side of slope block
// @param canUseSteepSlopes: Pedestrians and player motorcycles can use steep slopes (usually stairs)
// @param isAcrossLayers: Object corners are on two map layers
static bool IsSlopeSideOpen(int slopeType, eBlockFace face, bool canUseSteepSlopes, bool isAcrossLayers)
{
    // slopes go up to N, S, W, E: low end is always open; high end only when object already reaches layer above
    static const eBlockFace lowEnd[] = { eBlockFace_S, eBlockFace_N, eBlockFace_E, eBlockFace_W };
    static const eBlockFace highEnd[] = { eBlockFace_N, eBlockFace_S, eBlockFace_W, eBlockFace_E };

    int direction = 0;
    bool isLowPart = false;
    bool isHighPart = false;
    if (slopeType >= 9 && slopeType <= 40) // 7 degrees, 8 parts
    {
        direction = (slopeType - 9) / 8;
        isLowPart = ((slopeType - 9) % 8) == 0;
        isHighPart = !isLowPart;
    }
    else if (slopeType >= 1 && slopeType <= 8 && canUseSteepSlopes) // 26 degrees, 2 parts
    {
        direction = (slopeType - 1) / 2;
        isLowPart = ((slopeType - 1) % 2) == 0;
        isHighPart = !isLowPart;
    }
    else if (slopeType >= 41 && slopeType <= 44 && canUseSteepSlopes) // 45 degrees, 1 part
    {
        direction = slopeType - 41;
        isLowPart = true;
        isHighPart = true;
    }
    return (isLowPart && (face == lowEnd[direction])) || (isHighPart && isAcrossLayers && (face == highEnd[direction]));
}

static void CreateMapWallFixture(b2Body* mapBody, const b2Shape& shape, b2FixtureData_map fixtureData)
{
    b2FixtureDef b2fixtureDef;
    b2fixtureDef.density = 0.0f;
    b2fixtureDef.shape = &shape;
    b2fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(fixtureData.mAsPointer);
    b2fixtureDef.filter.categoryBits = CollisionGroup_Wall;

    b2Fixture* b2fixture = mapBody->CreateFixture(&b2fixtureDef);
    debug_assert(b2fixture);
}

//////////////////////////////////////////////////////////////////////////

inline bool CheckCollisionGroup(const b2Fixture* fixture, CollisionGroup collisionGroup)
{
    const b2Filter& filterData = fixture->GetFilterData();
    return (filterData.categoryBits & collisionGroup) > 0;
}

//////////////////////////////////////////////////////////////////////////

PhysicsManager gPhysics;

PhysicsManager::PhysicsManager()
    : mBox2MapBody()
    , mBox2World()
    , mGravity()
{
}

void PhysicsManager::EnterWorld()
{
    b2Vec2 gravity {0.0f, 0.0f};
    mBox2World = new b2World(gravity);
    mBox2World->SetContactListener(this);

    mSimulationStepTime = 1.0f / std::max(gCvarPhysicsFramerate.mValue, 1.0f);
    mGravity = Convert::MapUnitsToMeters(0.5f);

    CreateMapCollisionShape();
}

void PhysicsManager::ClearWorld()
{
    if (mBox2MapBody)
    {
        mBox2World->DestroyBody(mBox2MapBody);
        mBox2MapBody = nullptr;
    }
    SafeDelete(mBox2World);
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
    const int positionIterations = 4;

    // fixed update
    for (size_t i = 0, NumElements = mBodiesList.size(); i < NumElements; ++i)
    {
        PhysicsBody* currObjectBody = mBodiesList[i];
        if (!currObjectBody->CheckFlags(PhysicsBodyFlags_Disabled))
        {
            GameObject* currGameObject = currObjectBody->mGameObject;
            currGameObject->SimulationStep();
            CheckPedestrianStep(currObjectBody);
        }
    }

    // drop old contacts before new simulation frame
    for (PhysicsBody* currObjectBody: mBodiesList)
    {
        GameObject* currGameObject = currObjectBody->mGameObject;
        currObjectBody->SetAwake(true); // force contacting bodies awaken
        currGameObject->ClearContacts();
    }

    mContactsImpactSpeed.clear();
    mBox2World->Step(mSimulationStepTime, velocityIterations, positionIterations);

    // process y position
    for (PhysicsBody* currObjectBody: mBodiesList)
    {
        GameObject* currGameObject = currObjectBody->mGameObject;
        if (currGameObject->IsAttachedToObject() || currObjectBody->CheckFlags(PhysicsBodyFlags_Disabled))
            continue;

        UpdateHeightPosition(currObjectBody);
    }

    DispatchCollisionEvents();

    // sync transform
    for (PhysicsBody* currObjectBody: mBodiesList)
    {
        GameObject* currGameObject = currObjectBody->mGameObject;
        if (!currGameObject->IsAttachedToObject())
        {
            currGameObject->SyncPhysicsTransform();
        }
    }
}

PhysicsBody* PhysicsManager::CreateBody(GameObject* gameObject, PhysicsBodyFlags flags)
{
    debug_assert(!IsSimulationStepInProgress());

    debug_assert(gameObject);
    PhysicsBody* physicsBody = gPhysicsBodiesPool.create(gameObject, flags);
    debug_assert(physicsBody);

    mBodiesList.push_back(physicsBody);
    return physicsBody;
}

PhysicsBody* PhysicsManager::CreateBody(GameObject* gameObject, const CollisionShape& shapeData, const PhysicsMaterial& shapeMaterial,
    CollisionGroup collisionGroup,
    CollisionGroup collidesWith, ColliderFlags colliderFlags, PhysicsBodyFlags bodyFlags)
{
    PhysicsBody* body = CreateBody(gameObject, bodyFlags);
    debug_assert(body);
    if (body)
    {
        Collider* collider = body->AddCollider(0, shapeData, shapeMaterial, collisionGroup, collidesWith, colliderFlags);
        debug_assert(collider);
    }
    return body;
}

void PhysicsManager::CreateMapCollisionShape()
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(nullptr); // make sure userdata is nullptr

    mBox2MapBody = mBox2World->CreateBody(&bodyDef);
    debug_assert(mBox2MapBody);

    int numFixtures = 0;
    // for each block create fixture
    for (int x = 0; x < MAP_DIMENSIONS; ++x)
    {
        for (int y = 0; y < MAP_DIMENSIONS; ++y)
        {
            for (int layer = 0; layer < MAP_LAYERS_COUNT; ++layer)
            {
                const MapBlockInfo* blockData = gGameMap.GetBlockInfo(x, y, layer);
                debug_assert(blockData);

                if (blockData->mGroundType != eGroundType_Building)
                    continue;

                // checek blox is inner
                {
                    const MapBlockInfo* neighbourE = gGameMap.GetBlockInfo(x + 1, y, layer);
                    const MapBlockInfo* neighbourW = gGameMap.GetBlockInfo(x - 1, y, layer);
                    const MapBlockInfo* neighbourN = gGameMap.GetBlockInfo(x, y - 1, layer);
                    const MapBlockInfo* neighbourS = gGameMap.GetBlockInfo(x, y + 1, layer);

                    auto is_walkable = [](eGroundType gtype)
                    {
                        return gtype == eGroundType_Field || gtype == eGroundType_Pawement || gtype == eGroundType_Road || gtype == eGroundType_Water;
                    };

                    if (!is_walkable(neighbourE->mGroundType) && !is_walkable(neighbourW->mGroundType) &&
                        !is_walkable(neighbourN->mGroundType) && !is_walkable(neighbourS->mGroundType))
                    {
                        continue; // just ignore this block
                    }
                }

                b2PolygonShape b2shapeDef;

                glm::vec2 shapeCenter (x + 0.5f, y + 0.5f);
                shapeCenter = Convert::MapUnitsToMeters(shapeCenter);

                glm::vec2 shapeLength (0.5f, 0.5f);
                shapeLength = Convert::MapUnitsToMeters(shapeLength);

                b2shapeDef.SetAsBox(shapeLength.x, shapeLength.y, convert_vec2(shapeCenter), 0.0f);

                b2FixtureData_map fixtureData;
                fixtureData.mX = x;
                fixtureData.mZ = y;

                b2FixtureDef b2fixtureDef;
                b2fixtureDef.density = 0.0f;
                b2fixtureDef.shape = &b2shapeDef;
                b2fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(fixtureData.mAsPointer);
                b2fixtureDef.filter.categoryBits = CollisionGroup_MapBlock;

                b2Fixture* b2fixture = mBox2MapBody->CreateFixture(&b2fixtureDef);
                debug_assert(b2fixture);

                ++numFixtures;
                break; // single fixture per block column
            }
        }
    }

    // walls on slope sides so slopes can be entered only from their ends
    for (int layer = 0; layer < MAP_LAYERS_COUNT; ++layer)
    {
        for (int x = 0; x < MAP_DIMENSIONS; ++x)
        {
            for (int y = 0; y < MAP_DIMENSIONS; ++y)
            {
                const MapBlockInfo* blockData = gGameMap.GetBlockInfo(x, y, layer);
                if (blockData->mSlopeType == 0 || blockData->mGroundType == eGroundType_Air)
                    continue;

                for (eBlockFace face: {eBlockFace_W, eBlockFace_E, eBlockFace_N, eBlockFace_S})
                {
                    int neighbourX = x + (face == eBlockFace_E) - (face == eBlockFace_W);
                    int neighbourY = y + (face == eBlockFace_S) - (face == eBlockFace_N);
                    if (neighbourX < 0 || neighbourX >= MAP_DIMENSIONS || neighbourY < 0 || neighbourY >= MAP_DIMENSIONS)
                        continue;

                    if (gGameMap.GetBlockInfo(neighbourX, neighbourY, layer)->mSlopeType)
                        continue;

                    glm::ivec2 pointA (x * PIXELS_PER_MAP_UNIT, y * PIXELS_PER_MAP_UNIT);
                    glm::ivec2 pointB = pointA;
                    switch (face)
                    {
                        case eBlockFace_W: pointA.x = pointB.x = pointA.x - 1; pointB.y += PIXELS_PER_MAP_UNIT; break;
                        case eBlockFace_E: pointA.x = pointB.x = pointA.x + PIXELS_PER_MAP_UNIT + 1; pointB.y += PIXELS_PER_MAP_UNIT; break;
                        case eBlockFace_N: pointA.y = pointB.y = pointA.y - 1; pointB.x += PIXELS_PER_MAP_UNIT; break;
                        case eBlockFace_S: pointA.y = pointB.y = pointA.y + PIXELS_PER_MAP_UNIT + 1; pointB.x += PIXELS_PER_MAP_UNIT; break;
                        default: break;
                    }

                    b2EdgeShape b2shapeDef;
                    b2shapeDef.SetTwoSided(convert_vec2(Convert::PixelsToMeters(pointA)), convert_vec2(Convert::PixelsToMeters(pointB)));

                    b2FixtureData_map fixtureData;
                    fixtureData.mX = x;
                    fixtureData.mZ = y;
                    fixtureData.mLayer = layer;
                    fixtureData.mFace = face;
                    CreateMapWallFixture(mBox2MapBody, b2shapeDef, fixtureData);
                }
            }
        }
    }
}

void PhysicsManager::DestroyBody(PhysicsBody* physicsBody)
{
    debug_assert(!IsSimulationStepInProgress());

    debug_assert(physicsBody);
    if (physicsBody == nullptr)
        return;

    GameObject* gameObject = physicsBody->mGameObject;
    debug_assert(gameObject);
    if (gameObject)
    {
        cxx::erase_elements(mBodiesList, physicsBody);
        // remove contacts
        for (const Contact& currContact: gameObject->mObjectsContacts)
        {
            currContact.mThatObject->UnregisterContactsWithObject(gameObject);
        }
        gameObject->ClearContacts();
    }

    gPhysicsBodiesPool.destroy(physicsBody);
}

void PhysicsManager::BeginContact(b2Contact* contact)
{
    // do nothing
}

void PhysicsManager::EndContact(b2Contact* contact)
{
    // do nothing
}

void PhysicsManager::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    bool enableCollisionResponse = true;
    b2Fixture* objectWithMapFixture = nullptr;

    // check map collision
    if (CheckCollisionGroup(fixtureA, CollisionGroup_MapBlock | CollisionGroup_Wall))
    {
        // assume fixtureB is game object collider
        enableCollisionResponse = ShouldCollide_ObjectWithMap(contact, fixtureB, fixtureA);
        objectWithMapFixture = fixtureB;
    }
    else if (CheckCollisionGroup(fixtureB, CollisionGroup_MapBlock | CollisionGroup_Wall))
    {
        // assume fixtureA is game object collider
        enableCollisionResponse = ShouldCollide_ObjectWithMap(contact, fixtureA, fixtureB);
        objectWithMapFixture = fixtureA;
    }
    else // object vs object
    {
        enableCollisionResponse = ShouldCollide_Objects(contact, fixtureA, fixtureB);
    }
    contact->SetEnabled(enableCollisionResponse);

    // cars bounce from walls
    if (enableCollisionResponse && objectWithMapFixture && CheckCollisionGroup(objectWithMapFixture, CollisionGroup_Car))
    {
        contact->SetRestitution(gGameParams.mCarWallRestitution);
        contact->SetRestitutionThreshold(gGameParams.mCarMinImpactSpeed);
    }

    // velocities are not changed by collision response yet, so impact speed is correct here
    if (enableCollisionResponse)
    {
        mContactsImpactSpeed[contact] = ComputeContactImpactSpeed(contact);
    }
}

float PhysicsManager::ComputeContactImpactSpeed(b2Contact* contact) const
{
    int pointCount = contact->GetManifold()->pointCount;
    if (pointCount < 1)
        return 0.0f;

    b2Body* bodyA = contact->GetFixtureA()->GetBody();
    b2Body* bodyB = contact->GetFixtureB()->GetBody();

    b2WorldManifold wmanifold;
    contact->GetWorldManifold(&wmanifold);

    // highest closing speed at the contact points
    float impactSpeed = 0.0f;
    for (int icurr = 0; icurr < pointCount; ++icurr)
    {
        b2Vec2 velocityA = bodyA->GetLinearVelocityFromWorldPoint(wmanifold.points[icurr]);
        b2Vec2 velocityB = bodyB->GetLinearVelocityFromWorldPoint(wmanifold.points[icurr]);
        float normalSpeed = b2Dot(velocityB - velocityA, wmanifold.normal);
        impactSpeed = b2Max(impactSpeed, -normalSpeed);
    }
    return impactSpeed;
}

float PhysicsManager::GetContactImpactSpeed(b2Contact* contact) const
{
    auto foundIter = mContactsImpactSpeed.find(contact);
    if (foundIter == mContactsImpactSpeed.end())
        return 0.0f;

    return foundIter->second;
}

void PhysicsManager::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
    if (impulse->count < 1 || contact->GetManifold()->pointCount < 1)
        return;

    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    // check map collision
    if (CheckCollisionGroup(fixtureA, CollisionGroup_MapBlock | CollisionGroup_Wall))
    {
        // assume fixtureB is game object collider
        HandleCollision_ObjectWithMap(fixtureB, fixtureA, contact, impulse);
        return;
    }

    if (CheckCollisionGroup(fixtureB, CollisionGroup_MapBlock | CollisionGroup_Wall))
    {
        // assume fixtureA is game object collider
        HandleCollision_ObjectWithMap(fixtureA, fixtureB, contact, impulse);
        return;
    }

    // object vs object
    {
        HandleCollision_Objects(fixtureA, fixtureB, contact, impulse);
        return;
    }
}

void PhysicsManager::UpdateHeightPosition(PhysicsBody* physicsBody)
{
    GameObject* gameObject = physicsBody->mGameObject;
    debug_assert(gameObject);

    if (gameObject->IsAttachedToObject())
    {
        debug_assert(false);
        return;
    }

    bool isCar = gameObject->IsVehicleClass();
    bool isPedestrian = gameObject->IsPedestrianClass();
    float groundHeight = (isPedestrian || isCar) ?
        GetGroundHeightUnder(physicsBody->GetPosition2(), physicsBody->mPositionY) :
        gGameMap.GetHeightAtPosition(physicsBody->GetPosition(), false);

    if (physicsBody->mWaterContact)
    {
        // drowning player can walk out of water
        bool isDrowning = isPedestrian && static_cast<Pedestrian*>(gameObject)->IsDrowning();
        glm::vec3 groundPosition(physicsBody->GetPosition().x, groundHeight, physicsBody->GetPosition().z);
        if (!isDrowning || gGameMap.IsWaterAtPosition(groundPosition))
            return;

        physicsBody->mWaterContact = false;
    }

    float prevHeight = physicsBody->mPositionY;
    if (physicsBody->mFalling)
    {
        // whether falling ends
        bool isLanding = false;
        if (physicsBody->mFallHoverTime > 0.0f)
        {
            // body stays at same height, it lands only on higher ground
            physicsBody->mFallHoverTime -= mSimulationStepTime;
            if (groundHeight > physicsBody->mPositionY)
            {
                physicsBody->mPositionY = groundHeight;
                isLanding = true;
            }
        }
        else if (isPedestrian && (((physicsBody->mPositionY - groundHeight) <= gGameParams.mPedestrianFallStartHeight) ||
            ((physicsBody->mFallStartHeight - physicsBody->mPositionY) >= gGameParams.mPedestrianFallMaxDistance)))
        {
            // close to ground, or died mid-air
            if (groundHeight <= physicsBody->mPositionY + gGameParams.mPedestrianStepHeight)
            {
                physicsBody->mPositionY = groundHeight;
            }
            isLanding = true;
        }
        else
        {
            float fallSpeed = isCar ? gGameParams.mCarFallSpeed : (isPedestrian ? gGameParams.mPedestrianFallSpeed : mGravity);
            physicsBody->mPositionY = std::max(groundHeight, physicsBody->mPositionY - (fallSpeed * mSimulationStepTime));
            isLanding = (physicsBody->mPositionY == groundHeight);
        }
        if (isLanding)
        {
            if (gGameMap.IsWaterAtPosition(physicsBody->GetPosition()))
            {
                HandleFallsOnWater(physicsBody);
            }
            else
            {
                HandleFallsOnGround(physicsBody);
            }
        }
    }
    else
    {
        float fallThresholdDistance = isPedestrian ? gGameParams.mPedestrianFallStartHeight : 0.1f;
        bool onTheGround = fabsf(groundHeight - physicsBody->mPositionY) <= fallThresholdDistance;
        if (isCar)
        {
            // car follows ground that goes down a little
            glm::vec2 nextPosition = physicsBody->GetPosition2() + (physicsBody->GetLinearVelocity() * gGameParams.mCarFallLookAheadTime);
            float nextGroundHeight = GetGroundHeightUnder(nextPosition, physicsBody->mPositionY);
            onTheGround = (physicsBody->mPositionY - std::min(groundHeight, nextGroundHeight)) <= gGameParams.mCarFallStartHeight;
        }
        if (onTheGround)
        {
            physicsBody->mPositionY = groundHeight;
            bool canDrown = isCar || (isPedestrian && !static_cast<Pedestrian*>(gameObject)->IsEnteringOrExitingCar());
            if (canDrown && gGameMap.IsWaterAtPosition(physicsBody->GetPosition()))
            {
                HandleFallsOnWater(physicsBody);
            }
        }
        else if (!isPedestrian || (groundHeight < physicsBody->mPositionY)) // start falling
        {
            if (gGameCheatsWindow.mEnableGravity)
            {
                HandleFallingStarts(physicsBody);
            }
        }
    }

    if (prevHeight != physicsBody->mPositionY)
    {
        // force body awake
        physicsBody->SetAwake(true);
    }
}

void PhysicsManager::CheckPedestrianStep(PhysicsBody* physicsBody)
{
    GameObject* gameObject = physicsBody->mGameObject;
    if (!gameObject->IsPedestrianClass() || gameObject->IsAttachedToObject() || physicsBody->mFalling)
        return;

    glm::vec2 velocity = physicsBody->GetLinearVelocity();
    float speed = glm::length(velocity);
    if (speed == 0.0f || speed > gGameParams.mPedestrianStepBlockSpeed)
        return;

    glm::vec2 nextPosition = physicsBody->GetPosition2() + (velocity * mSimulationStepTime);
    if (GetGroundHeightUnder(nextPosition, physicsBody->mPositionY) > physicsBody->mPositionY + gGameParams.mPedestrianStepHeight)
    {
        physicsBody->SetLinearVelocity(glm::vec2());
    }
}

float PhysicsManager::GetGroundHeightUnder(const glm::vec2& position, float height) const
{
    float searchHeight = height + gGameParams.mGameGroundSearchHeight;
    return gGameMap.GetGroundHeight(position, (int) std::floor(Convert::MetersToMapUnits(searchHeight)), false);
}

void PhysicsManager::GetCornersMapLayers(GameObject* gameObject, int& topLayer, int& bottomLayer) const
{
    PhysicsBody* physicsBody = gameObject->mPhysicsBody;
    const float margin = Convert::PixelsToMeters(4);
    const glm::vec2 signs[4] = { {-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f} };

    glm::vec2 corners[4];
    if (Vehicle* carObject = ToVehicle(gameObject))
    {
        carObject->GetChassisCornersLocal(corners);
        for (int icorner = 0; icorner < 4; ++icorner)
        {
            corners[icorner] = physicsBody->GetWorldPoint(corners[icorner] + signs[icorner] * margin);
        }
    }
    else
    {
        const float halfBox = gGameParams.mPedestrianBoundsSphereRadius + margin;
        for (int icorner = 0; icorner < 4; ++icorner)
        {
            corners[icorner] = physicsBody->GetPosition2() + signs[icorner] * halfBox;
        }
    }

    for (int icorner = 0; icorner < 4; ++icorner)
    {
        float groundHeight = GetGroundHeightUnder(corners[icorner], physicsBody->mPositionY);
        int layer = (int) std::floor(Convert::MetersToMapUnits(groundHeight));
        topLayer = (icorner == 0) ? layer : std::max(topLayer, layer);
        bottomLayer = (icorner == 0) ? layer : std::min(bottomLayer, layer);
    }
}

int PhysicsManager::GetObjectMapLayer(GameObject* gameObject) const
{
    if (gameObject->IsPedestrianClass())
    {
        return (int) std::floor(Convert::MetersToMapUnits(gameObject->mPhysicsBody->mPositionY));
    }
    if (gameObject->IsVehicleClass())
    {
        int topLayer = 0;
        int bottomLayer = 0;
        GetCornersMapLayers(gameObject, topLayer, bottomLayer);
        return topLayer;
    }
    float height = gGameMap.GetHeightAtPosition(gameObject->mPhysicsBody->GetPosition());
    return (int) (Convert::MetersToMapUnits(height) + 0.5f);
}

void PhysicsManager::QueryObjectsLinecast(const glm::vec2& pointA, const glm::vec2& pointB, PhysicsQueryResult& outputResult, CollisionGroup collisionMask) const
{
    outputResult.Clear();

    collisionMask = collisionMask & ~(CollisionGroup_MapBlock | CollisionGroup_Wall); // ignore map
    if (collisionMask == CollisionGroup_None)
        return;

    struct _raycast_callback: public b2RayCastCallback
    {
    public:
        _raycast_callback(PhysicsQueryResult& out, CollisionGroup collisionMask)
            : mOutput(out)
            , mCollisionMask(collisionMask)
        {
        }
	    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
        {
            if (mOutput.IsFull())
                return 0.0f;

            const b2Filter& filterData = fixture->GetFilterData();
            if ((filterData.categoryBits & mCollisionMask) > 0)
            {
                PhysicsQueryElement& currHit = mOutput.mElements[mOutput.mElementsCount];
                currHit.mPhysicsObject = b2Fixture_get_physics_body(fixture);
                if (currHit.mPhysicsObject)
                {
                    ++mOutput.mElementsCount;

                    currHit.mIntersectionPoint = convert_vec2(point);
                    currHit.mNormal = convert_vec2(point);
                }
            }
            return 1.0f;
        }
    public:
        PhysicsQueryResult& mOutput;
        CollisionGroup mCollisionMask;
    };

    _raycast_callback raycast_callback {outputResult, collisionMask};
    b2Vec2 p1 = convert_vec2(pointA);
    b2Vec2 p2 = convert_vec2(pointB);
    mBox2World->RayCast(&raycast_callback, p1, p2);
}

void PhysicsManager::QueryObjectsWithinBox(const glm::vec2& center, const glm::vec2& extents, PhysicsQueryResult& outputResult, CollisionGroup collisionMask) const
{
    outputResult.Clear();

    collisionMask = collisionMask & ~(CollisionGroup_MapBlock | CollisionGroup_Wall); // ignore map
    if (collisionMask == CollisionGroup_None)
        return;

    struct _query_callback: public b2QueryCallback
    {
    public:
        _query_callback(PhysicsQueryResult& out, CollisionGroup collisionMask)
            : mOutput(out)
            , mCollisionMask(collisionMask)
        {
        }
        bool ReportFixture(b2Fixture* fixture) override
        {
            if (mOutput.IsFull())
                return false;

            const b2Filter& filterData = fixture->GetFilterData();
            if ((filterData.categoryBits & mCollisionMask) > 0)
            {
                PhysicsQueryElement& currHit = mOutput.mElements[mOutput.mElementsCount];
                currHit.mPhysicsObject = b2Fixture_get_physics_body(fixture);
                if (currHit.mPhysicsObject)
                {
                    ++mOutput.mElementsCount;
                }
            }
            return true;
        }
    public:
        PhysicsQueryResult& mOutput;
        CollisionGroup mCollisionMask;
    };
    _query_callback query_callback {outputResult, collisionMask};

    b2AABB aabb;
    aabb.lowerBound.x = (center.x - extents.x);
    aabb.lowerBound.y = (center.y - extents.y);
    aabb.upperBound.x = (center.x + extents.x);
    aabb.upperBound.y = (center.y + extents.y);
    mBox2World->QueryAABB(&query_callback, aabb);
}

bool PhysicsManager::IsSimulationStepInProgress() const
{
    return mBox2World->IsLocked();
}

bool PhysicsManager::ShouldCollide_ObjectWithMap(b2Contact* contact, b2Fixture* objectFixture, b2Fixture* mapFixture) const
{
    GameObject* gameObject = b2Fixture_get_game_object(objectFixture);

    debug_assert(gameObject);
    debug_assert(!gameObject->IsAttachedToObject());

    if (gameObject->IsMarkedForDeletion())
        return false;

    debug_assert(gameObject->mPhysicsBody);
    if (gameObject->mPhysicsBody->CheckFlags(PhysicsBodyFlags_Disabled))
        return false;

    Pedestrian* pedestrian = ToPedestrian(gameObject);
    Vehicle* carObject = ToVehicle(gameObject);
    b2FixtureData_map fxdata = (b2FixtureData_map*) mapFixture->GetUserData().pointer;
    bool isWall = CheckCollisionGroup(mapFixture, CollisionGroup_Wall);

    if (pedestrian)
    {
        if (pedestrian->mPhysicsBody->mFalling)
            return false;

        glm::vec2 position = Convert::MetersToMapUnits(pedestrian->mPhysicsBody->GetPosition2());
        const MapBlockInfo* blockData = gGameMap.GetBlockInfo((int) std::floor(position.x), (int) std::floor(position.y), GetObjectMapLayer(gameObject));
        if (blockData->mSlopeType && (!isWall || !pedestrian->IsHumanPlayerCharacter()))
            return false;
    }

    if (!isWall)
    {
        const MapBlockInfo* blockData = gGameMap.GetBlockInfo(fxdata.mX, fxdata.mZ, GetObjectMapLayer(gameObject));
        return (blockData->mGroundType == eGroundType_Building);
    }

    if (!pedestrian && !carObject)
        return false;

    int topLayer = 0;
    int bottomLayer = 0;
    GetCornersMapLayers(gameObject, topLayer, bottomLayer);

    // find slope from highest corner
    glm::vec2 blockCenter = Convert::MapUnitsToMeters(glm::vec2(fxdata.mX + 0.5f, fxdata.mZ + 0.5f));
    float slopeHeight = gGameMap.GetGroundHeight(blockCenter, topLayer, false);
    if ((int) Convert::MetersToMapUnits(slopeHeight) != fxdata.mLayer)
        return false;

    const MapBlockInfo* blockData = gGameMap.GetBlockInfo(fxdata.mX, fxdata.mZ, fxdata.mLayer);
    // only player can use motorcycles on steep slopes (stairs)
    bool canUseSteepSlopes = !carObject;
    if (carObject && (carObject->mCarInfo->mClassID == eVehicleClass_Motorcycle))
    {
        Pedestrian* carDriver = carObject->GetCarDriver();
        canUseSteepSlopes = carDriver && carDriver->IsHumanPlayerCharacter();
    }
    return !IsSlopeSideOpen(blockData->mSlopeType, (eBlockFace) fxdata.mFace, canUseSteepSlopes, topLayer != bottomLayer);
}

bool PhysicsManager::ShouldCollide_Objects(b2Contact* box2contact, b2Fixture* fixtureA, b2Fixture* fixtureB) const
{
    GameObject* gameObjectA = b2Fixture_get_game_object(fixtureA);
    GameObject* gameObjectB = b2Fixture_get_game_object(fixtureB);

    debug_assert(gameObjectA);
    debug_assert(gameObjectB);
    debug_assert(gameObjectA != gameObjectB);

    if (gameObjectA->IsMarkedForDeletion() || gameObjectB->IsMarkedForDeletion())
        return false;

    debug_assert(gameObjectA->mPhysicsBody);
    debug_assert(gameObjectB->mPhysicsBody);

    if (gameObjectA->mPhysicsBody->CheckFlags(PhysicsBodyFlags_Disabled))
        return false;

    if (gameObjectB->mPhysicsBody->CheckFlags(PhysicsBodyFlags_Disabled))
        return false;

    // todo: check y positions!
    float objectA_posY = gameObjectA->mPhysicsBody->mPositionY;
    float objectB_posY = gameObjectB->mPhysicsBody->mPositionY;

    // todo: this is temporary implementation!
    if (fabs(objectA_posY - objectB_posY) > 2.0f)
        return false;

    if (gameObjectA->IsAttachedToObject() || gameObjectB->IsAttachedToObject())
    {
        if (gameObjectA->IsSameHierarchy(gameObjectB))
            return false;
    }

    bool shouldCollide = gameObjectA->ShouldCollide(gameObjectB) && gameObjectB->ShouldCollide(gameObjectA);

    if (!shouldCollide)
    {
        // register contact between objects
        Contact objectsContact;

        objectsContact.SetupWithBox2Data(box2contact, fixtureA, fixtureB);
        gameObjectA->RegisterContact(objectsContact);

        objectsContact.SetupWithBox2Data(box2contact, fixtureB, fixtureA);
        gameObjectB->RegisterContact(objectsContact);
    }
    return shouldCollide;
}

void PhysicsManager::HandleCollision_ObjectWithMap(b2Fixture* objectFixture, b2Fixture* mapFixture, b2Contact* contact, const b2ContactImpulse* impulse)
{
    GameObject* gameObject = b2Fixture_get_game_object(objectFixture);
    debug_assert(gameObject);

    b2FixtureData_map fxdata = (b2FixtureData_map*) mapFixture->GetUserData().pointer;
    int mapLayer = CheckCollisionGroup(mapFixture, CollisionGroup_Wall) ? fxdata.mLayer : GetObjectMapLayer(gameObject);

    // queue collision event
    mObjectsCollisionList.emplace_back();

    CollisionEvent& collisionEvent = mObjectsCollisionList.back();
    collisionEvent.mBox2Contact = contact;
    collisionEvent.mBox2FixtureA = objectFixture;
    collisionEvent.mImpactSpeed = GetContactImpactSpeed(contact);
    collisionEvent.mMapBlockInfo = gGameMap.GetBlockInfo(fxdata.mX, fxdata.mZ, mapLayer);
    debug_assert(collisionEvent.mMapBlockInfo);

    if (Vehicle* carObject = ToVehicle(gameObject))
    {
        HandleCollision_CarVsMap(carObject, contact, impulse);
        return;
    }
}

void PhysicsManager::HandleCollision_Objects(b2Fixture* fixtureA, b2Fixture* fixtureB, b2Contact* contact, const b2ContactImpulse* impulse)
{
    // queue collision event
    mObjectsCollisionList.emplace_back();

    CollisionEvent& collisionEvent = mObjectsCollisionList.back();
    collisionEvent.mBox2Contact = contact;
    collisionEvent.mBox2FixtureA = fixtureA;
    collisionEvent.mBox2FixtureB = fixtureB;
    collisionEvent.mImpactSpeed = GetContactImpactSpeed(contact);

    // handle collision
    GameObject* objectA = b2Fixture_get_game_object(fixtureA);
    GameObject* objectB = b2Fixture_get_game_object(fixtureB);
    debug_assert(objectA && objectB);

    if (IsSameClass(objectA, objectB, eGameObjectClass_Car))
    {
        Vehicle* carA = ToVehicle(objectA);
        Vehicle* carB = ToVehicle(objectB);

        HandleCollision_CarVsCar(carA, carB, contact, impulse);
        return;
    }
}

void PhysicsManager::HandleCollision_CarVsCar(Vehicle* carA, Vehicle* carB, b2Contact* contact, const b2ContactImpulse* impulse)
{
    debug_assert(carA && carB);

    if (gParticleManager.IsCarSparksEffectEnabled())
    {
        int pointCount = contact->GetManifold()->pointCount;
        float impact = 0.0f;
        for (int i = 0; i < pointCount; ++i)
        {
            impact = b2Max(impact, impulse->normalImpulses[i]);
        }
        b2WorldManifold wmanifold;
        contact->GetWorldManifold(&wmanifold);

        if (impact > gGameParams.mSparksOnCarsContactThreshold)
        {
            glm::vec3 contactPoint(wmanifold.points->x, carA->mPhysicsBody->mPositionY, wmanifold.points->y);
            glm::vec2 velocity2 = glm::normalize(
                carA->mPhysicsBody->GetLinearVelocity() +
                carB->mPhysicsBody->GetLinearVelocity());
            glm::vec3 velocity = -glm::vec3(velocity2.x, 0.0f, velocity2.y) * 1.8f;
            gParticleManager.StartCarSparks(contactPoint, velocity, 3);
        }
    }
}

void PhysicsManager::HandleCollision_CarVsMap(Vehicle* car, b2Contact* contact, const b2ContactImpulse* impulse)
{
    debug_assert(car);

    int pointCount = contact->GetManifold()->pointCount;
    float impact = 0.0f;
    for (int i = 0; i < pointCount; ++i)
    {
        impact = b2Max(impact, impulse->normalImpulses[i]);
    }

    if (gParticleManager.IsCarSparksEffectEnabled())
    {
        b2WorldManifold wmanifold;
        contact->GetWorldManifold(&wmanifold);

        if (impact > gGameParams.mSparksOnCarsContactThreshold)
        {
            glm::vec3 contactPoint(wmanifold.points->x, car->mPhysicsBody->mPositionY, wmanifold.points->y);
            glm::vec2 velocity2 = glm::normalize(car->mPhysicsBody->GetLinearVelocity());
            glm::vec3 velocity = -glm::vec3(velocity2.x, 0.0f, velocity2.y) * 1.8f;
            gParticleManager.StartCarSparks(contactPoint, velocity, 3);
        }
    }


    // sound
    if (impact > 700.0f)
    {
        //gAudioManager.StartSound(eSfxSampleType_Level, SfxLevel_CarCrash4, SfxFlags_RandomPitch, car->mTransform.mPosition);
    }

    // bounce

}

void PhysicsManager::ProcessInterpolation()
{
    float mixFactor = mSimulationTimeAccumulator / mSimulationStepTime;

    for (PhysicsBody* currBody: mBodiesList)
    {
        GameObject* gameObject = currBody->mGameObject;
        if (!gameObject->IsAttachedToObject()) // interpolate roots
        {
            gameObject->InterpolateTransform(mixFactor);
        }
    }
}

void PhysicsManager::DispatchCollisionEvents()
{
    for (const CollisionEvent& currCollision: mObjectsCollisionList)
    {
        if (currCollision.mMapBlockInfo)
        {
            MapCollision collisionInfo;

            collisionInfo.SetupWithBox2Data(currCollision.mBox2Contact, currCollision.mBox2FixtureA, currCollision.mMapBlockInfo, currCollision.mImpactSpeed);
            collisionInfo.mThisObject->HandleCollisionWithMap(collisionInfo);
        }
        else
        {
            Collision collisionInfo;

            collisionInfo.SetupWithBox2Data(currCollision.mBox2Contact, currCollision.mBox2FixtureA, currCollision.mBox2FixtureB, currCollision.mImpactSpeed);
            collisionInfo.mContactInfo.mThisObject->HandleCollision(collisionInfo);

            collisionInfo.SetupWithBox2Data(currCollision.mBox2Contact, currCollision.mBox2FixtureB, currCollision.mBox2FixtureA, currCollision.mImpactSpeed);
            collisionInfo.mContactInfo.mThisObject->HandleCollision(collisionInfo);
        }
    }
    mObjectsCollisionList.clear();
}

void PhysicsManager::HandleFallingStarts(PhysicsBody* physicsBody)
{
    if (physicsBody->mFalling)
        return;

    physicsBody->mFalling = true;
    physicsBody->mFallStartHeight = physicsBody->mPositionY;

    physicsBody->mGameObject->HandleFallingStarts();
}

void PhysicsManager::HandleFallsOnGround(PhysicsBody* physicsBody)
{
    if (!physicsBody->mFalling)
        return;

    float fallDistance = physicsBody->mFallStartHeight - physicsBody->mPositionY;
    if (fallDistance < 0.0f)
    {
        fallDistance = 0.0f;
    }
    physicsBody->mFalling = false;
    physicsBody->mGameObject->HandleFallsOnGround(fallDistance);
}

void PhysicsManager::HandleFallsOnWater(PhysicsBody* physicsBody)
{
    if (physicsBody->mWaterContact)
        return;

    float fallDistance = physicsBody->mFallStartHeight - physicsBody->mPositionY;
    if (fallDistance < 0.0f)
    {
        fallDistance = 0.0f;
    }
    physicsBody->mWaterContact = true;
    physicsBody->mFalling = false;

    Vehicle* carObject = ToVehicle(physicsBody->mGameObject);
    if (!carObject || (carObject->mCarInfo->mClassID != eVehicleClass_Boat))
    {
        Decoration* splashEffect = gGameObjectsManager.CreateWaterSplash(physicsBody->GetPosition());
        if (splashEffect)
        {
            splashEffect->StartGameObjectSound(0, eSfxSampleType_Level, SfxLevel_WaterSplash, SfxFlags_None);
        }

        if (carObject)
        {
            glm::vec2 corners[4];
            carObject->GetChassisCorners(corners);
            for (const glm::vec2& currCorner: corners)
            {
                glm::vec3 cornerPosition(currCorner.x, physicsBody->mPositionY, currCorner.y);
                if (gGameMap.IsWaterAtPosition(cornerPosition))
                {
                    gGameObjectsManager.CreateWaterSplash(cornerPosition);
                }
            }
        }
    }
    physicsBody->mGameObject->HandleFallsOnWater(fallDistance);

    if (!physicsBody->mGameObject->IsPedestrianClass())
    {
        physicsBody->mPositionY -= Convert::MapUnitsToMeters(1.0f); // put it down
    }
}

float PhysicsManager::GetSimulationStepTime() const
{
    return mSimulationStepTime;
}
