#pragma once

#include "PhysicsDefs.h"
#include "GameDefs.h"
#include "PhysicsComponents.h"

// note that the physics only works with meter units (Mt) rather then map units

// this class manages physics and collision detections for map and objects
class PhysicsManager final: private b2ContactListener
{
public:
    PhysicsManager();

    bool InitPhysicsWorld();
    void FreePhysicsWorld();

    void UpdateFrame();

    // Create pedestrian specific physical body
    // @param object: Reference object
    // @param position: World position, meters
    // @param rotationAngle: Heading
    PedPhysicsBody* CreatePhysicsBody(Pedestrian* object, const glm::vec3& position, cxx::angle_t rotationAngle);

    // Create car specific physical body
    // @param object: Reference object
    // @param position: World position, meters
    // @param rotationAngle: Heading
    CarPhysicsBody* CreatePhysicsBody(Vehicle* object, const glm::vec3& position, cxx::angle_t rotationAngle);

    // Create projectile specific physical body
    // @param object: Reference object
    // @param position: World position, meters
    // @param rotationAngle: Heading
    ProjectilePhysicsBody* CreatePhysicsBody(Projectile* object, const glm::vec3& position, cxx::angle_t rotationAngle);

    // Free physics object
    // @param object: Object to destroy, pointer becomes invalid
    void DestroyPhysicsBody(PedPhysicsBody* object);
    void DestroyPhysicsBody(CarPhysicsBody* object);
    void DestroyPhysicsBody(ProjectilePhysicsBody* object);

    // query all physics objects that intersects with line
    // note that depth is ignored so pointA and pointB has only 2 components
    // @param pointA, pointB: Line of intersect points
    // @param aaboxCenter, aabboxExtents: AABBox area of intersections
    // @param outputResult: Output objects
    void QueryObjectsLinecast(const glm::vec2& pointA, const glm::vec2& pointB, PhysicsLinecastResult& outputResult) const;
    void QueryObjectsWithinBox(const glm::vec2& aaboxCenter, const glm::vec2& aabboxExtents, PhysicsQueryResult& outputResult) const;

private:
    // create level map body, used internally
    void CreateMapCollisionShape();

    // apply gravity forces and correct y coord for objects
    void FixedStepGravity();

    void ProcessSimulationStep(bool resetPreviousState);
    void ProcessInterpolation();

    // override b2ContactFilter
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

    // pre solve collisions
    bool HasCollisionPedVsPed(b2Contact* contact, PedPhysicsBody* pedA, PedPhysicsBody* pedB) const;
    bool HasCollisionCarVsCar(b2Contact* contact, CarPhysicsBody* carA, CarPhysicsBody* carB) const;
    bool HasCollisionPedVsMap(int mapx, int mapz, float height) const;
    bool HasCollisionCarVsMap(b2Contact* contact, b2Fixture* fixtureCar, int mapx, int mapz) const;
    bool HasCollisionPedVsCar(b2Contact* contact, PedPhysicsBody* ped, CarPhysicsBody* car) const;

    // post solve collisions
    void HandleContactPedVsCar(b2Contact* contact, float impulse, PedPhysicsBody* ped, CarPhysicsBody* car);

    bool ProcessSensorContact(b2Contact* contact, bool onBegin);

private:
    b2Body* mMapCollisionShape;
    b2World* mPhysicsWorld;

    float mSimulationTimeAccumulator;
    float mSimulationStepTime;

    float mGravityForce; // meters per second

    // bodies pools
    cxx::object_pool<PedPhysicsBody> mPedsBodiesPool;
    cxx::object_pool<CarPhysicsBody> mCarsBodiesPool;
    cxx::object_pool<ProjectilePhysicsBody> mProjectileBodiesPool;

    // bodies lists
    cxx::intrusive_list<PhysicsBody> mPedsBodiesList;
    cxx::intrusive_list<PhysicsBody> mCarsBodiesList;
    cxx::intrusive_list<PhysicsBody> mProjectileBodiesList;
};

extern PhysicsManager gPhysics;