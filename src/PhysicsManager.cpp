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
    };

    void* mAsPointer;
};

static_assert(sizeof(b2FixtureData_map) <= sizeof(void*), "Cannot pack data into pointer");

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
        }
    }

    // drop old contacts before new simulation frame
    for (PhysicsBody* currObjectBody: mBodiesList)
    {
        GameObject* currGameObject = currObjectBody->mGameObject;
        currObjectBody->SetAwake(true); // force contacting bodies awaken
        currGameObject->ClearContacts();
    }

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
                        return gtype == eGroundType_Field || gtype == eGroundType_Pawement || gtype == eGroundType_Road;
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

    // check map collision
    if (CheckCollisionGroup(fixtureA, CollisionGroup_MapBlock | CollisionGroup_Wall))
    {
        // assume fixtureB is game object collider
        enableCollisionResponse = ShouldCollide_ObjectWithMap(contact, fixtureB, fixtureA);
    }
    else if (CheckCollisionGroup(fixtureB, CollisionGroup_MapBlock | CollisionGroup_Wall))
    {
        // assume fixtureA is game object collider
        enableCollisionResponse = ShouldCollide_ObjectWithMap(contact, fixtureA, fixtureB);
    }
    else // object vs object
    {
        enableCollisionResponse = ShouldCollide_Objects(contact, fixtureA, fixtureB);
    }
    contact->SetEnabled(enableCollisionResponse);
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

    if (physicsBody->mWaterContact)
        return;

    float groundHeight = gGameMap.GetHeightAtPosition(physicsBody->GetPosition(), false);

    float prevHeight = physicsBody->mPositionY;
    if (physicsBody->mFalling)
    {
        // whether falling ends
        float fallingDistance = mGravity * mSimulationStepTime;
        physicsBody->mPositionY = std::max(groundHeight, physicsBody->mPositionY - fallingDistance);
        if (physicsBody->mPositionY == groundHeight)
        {
            // handle water contact
            float waterHeight = gGameMap.GetWaterLevelAtPosition2(physicsBody->GetPosition2());
            if (groundHeight <= waterHeight)
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
        const float fallThresholdDistance = 0.1f;
        bool onTheGround = fabsf(groundHeight - physicsBody->mPositionY) <= fallThresholdDistance;
        if (onTheGround)
        {
            physicsBody->mPositionY = groundHeight;
        }
        else // start falling
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

    float height = gGameMap.GetHeightAtPosition(gameObject->mPhysicsBody->GetPosition());
    int mapLayer = (int) (Convert::MetersToMapUnits(height) + 0.5f);

    // todo: this is temporary implementation

    b2FixtureData_map fxdata = (b2FixtureData_map*) mapFixture->GetUserData().pointer;
    const MapBlockInfo* blockData = gGameMap.GetBlockInfo(fxdata.mX, fxdata.mZ, mapLayer);
    return (blockData->mGroundType == eGroundType_Building);
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

    float height = gGameMap.GetHeightAtPosition(gameObject->mPhysicsBody->GetPosition());
    int mapLayer = (int) (Convert::MetersToMapUnits(height) + 0.5f);

    b2FixtureData_map fxdata = (b2FixtureData_map*) mapFixture->GetUserData().pointer;

    // queue collision event
    mObjectsCollisionList.emplace_back();

    CollisionEvent& collisionEvent = mObjectsCollisionList.back();
    collisionEvent.mBox2Impulse = *impulse;
    collisionEvent.mBox2Contact = contact;
    collisionEvent.mBox2FixtureA = objectFixture;
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
    collisionEvent.mBox2Impulse = *impulse;
    collisionEvent.mBox2Contact = contact;
    collisionEvent.mBox2FixtureA = fixtureA;
    collisionEvent.mBox2FixtureB = fixtureB;

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

            collisionInfo.SetupWithBox2Data(currCollision.mBox2Contact, currCollision.mBox2FixtureA, currCollision.mMapBlockInfo, &currCollision.mBox2Impulse);
            collisionInfo.mThisObject->HandleCollisionWithMap(collisionInfo);
        }
        else
        {
            Collision collisionInfo;

            collisionInfo.SetupWithBox2Data(currCollision.mBox2Contact, currCollision.mBox2FixtureA, currCollision.mBox2FixtureB, &currCollision.mBox2Impulse);
            collisionInfo.mContactInfo.mThisObject->HandleCollision(collisionInfo);

            collisionInfo.SetupWithBox2Data(currCollision.mBox2Contact, currCollision.mBox2FixtureB, currCollision.mBox2FixtureA, &currCollision.mBox2Impulse);
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

    // create splash effect
    if (Vehicle* carObject = ToVehicle(physicsBody->mGameObject))
    {
        // create effect
        glm::vec2 splashPoints[5];
        carObject->GetChassisCorners(splashPoints);
        splashPoints[4] = carObject->mPhysicsBody->GetPosition2();
        for (const glm::vec2& currPoint: splashPoints)
        {
            Decoration* splashEffect = gGameObjectsManager.CreateWaterSplash(glm::vec3(currPoint.x, carObject->mPhysicsBody->mPositionY, currPoint.y));
            debug_assert(splashEffect);
        }
    }
    else // peds and small objects
    {
        Decoration* splashEffect = gGameObjectsManager.CreateWaterSplash(physicsBody->GetPosition());
        debug_assert(splashEffect);
    }
    physicsBody->mGameObject->HandleFallsOnWater(fallDistance);

    physicsBody->mPositionY -= Convert::MapUnitsToMeters(1.0f); // put it down
}

float PhysicsManager::GetSimulationStepTime() const
{
    return mSimulationStepTime;
}
