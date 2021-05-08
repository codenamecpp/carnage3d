#pragma once

#include "PhysicsDefs.h"
#include "GameDefs.h"

// note that the physics only works with meter units (Mt) not map units

// this class manages physics and collision detections for map and objects
class PhysicsManager final: private b2ContactListener
{
    friend class PhysicsBody;

public:
    PhysicsManager();

    void EnterWorld();
    void ClearWorld();
    void UpdateFrame();

    bool IsSimulationStepInProgress() const;

    // Create physical body for game object
    // @param object: Reference object, cannot be null
    PhysicsBody* CreateBody(GameObject* gameObject, PhysicsBodyFlags flags);
    PhysicsBody* CreateBody(GameObject* gameObject, const CollisionShape& shapeData, const PhysicsMaterial& shapeMaterial,
        CollisionGroup collisionGroup,
        CollisionGroup collidesWith, ColliderFlags colliderFlags, PhysicsBodyFlags bodyFlags);

    void DestroyBody(PhysicsBody* physicsBody);

    // query physics objects
    // note that depth is ignored so pointA and pointB has only 2 components
    // @param pointA, pointB: Line of intersect points
    // @param aaboxCenter, aabboxExtents: AABBox area of intersections
    // @param outputResult: Output objects
    void QueryObjectsLinecast(const glm::vec2& pointA, const glm::vec2& pointB, PhysicsQueryResult& outputResult, CollisionGroup collisionMask) const;
    void QueryObjectsWithinBox(const glm::vec2& center, const glm::vec2& extents, PhysicsQueryResult& outputResult, CollisionGroup collisionMask) const;

private:
    // override b2ContactListener
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
    void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

    bool ShouldCollide_ObjectWithMap(b2Contact* contact, b2Fixture* objectFixture, b2Fixture* mapFixture) const;
    bool ShouldCollide_Objects(b2Contact* contact, b2Fixture* fixtureA, b2Fixture* fixtureB) const;

    void HandleCollision_ObjectWithMap(b2Fixture* objectFixture, b2Fixture* mapFixture, b2Contact* contact, const b2ContactImpulse* impulse);
    void HandleCollision_Objects(b2Fixture* fixtureA, b2Fixture* fixtureB, b2Contact* contact, const b2ContactImpulse* impulse);

    // collision handlers
    void HandleCollision_CarVsCar(Vehicle* carA, Vehicle* carB, b2Contact* contact, const b2ContactImpulse* impulse);
    void HandleCollision_CarVsMap(Vehicle* car, b2Contact* contact, const b2ContactImpulse* impulse);

    // create level map body, used internally
    void CreateMapCollisionShape();

    void ProcessInterpolation();
    void ProcessSimulationStep();
    void UpdateHeightPosition(PhysicsBody* physicsBody);

    void DispatchCollisionEvents();

    void HandleFallingStarts(PhysicsBody* physicsBody);
    void HandleFallsOnGround(PhysicsBody* physicsBody);
    void HandleFallsOnWater(PhysicsBody* physicsBody);

private:

    struct CollisionEvent
    {
    public:
        CollisionEvent() = default;

        // object vs map
        const MapBlockInfo* mMapBlockInfo = nullptr;
        // object vs object
        b2Fixture* mBox2FixtureA = nullptr;
        b2Fixture* mBox2FixtureB = nullptr;
        // common
        b2Contact* mBox2Contact = nullptr;
        b2ContactImpulse mBox2Impulse;
    };

private:

    b2Body* mBox2MapBody;
    b2World* mBox2World;

    float mSimulationTimeAccumulator;
    float mSimulationStepTime;

    float mGravity; // meters per second
    std::vector<PhysicsBody*> mBodiesList;

    std::vector<CollisionEvent> mObjectsCollisionList;
};

extern PhysicsManager gPhysics;