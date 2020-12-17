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

    void EnterWorld();
    void ClearWorld();
    void UpdateFrame();

    // Create pedestrian specific physical body
    // @param object: Reference object
    // @param position: World position, meters
    // @param rotationAngle: Heading
    PedPhysicsBody* CreatePhysicsObject(Pedestrian* object, const glm::vec3& position, cxx::angle_t rotationAngle);

    // Create car specific physical body
    // @param object: Reference object
    // @param position: World position, meters
    // @param rotationAngle: Heading
    CarPhysicsBody* CreatePhysicsObject(Vehicle* object, const glm::vec3& position, cxx::angle_t rotationAngle);

    // Create projectile specific physical body
    // @param object: Reference object
    // @param position: World position, meters
    // @param rotationAngle: Heading
    ProjectilePhysicsBody* CreatePhysicsObject(Projectile* object, const glm::vec3& position, cxx::angle_t rotationAngle);

    // Free physics object
    // @param object: Object to destroy, pointer becomes invalid
    void DestroyPhysicsObject(PedPhysicsBody* object);
    void DestroyPhysicsObject(CarPhysicsBody* object);
    void DestroyPhysicsObject(ProjectilePhysicsBody* object);

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
    void ProcessGravityStep();
    void ProcessGravityStep(CarPhysicsBody* body);
    void ProcessGravityStep(PedPhysicsBody* body);

    void ProcessSimulationStep();
    void ProcessInterpolation();

    // override b2ContactFilter
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

    // pre solve collisions
    bool HasCollisionPedVsPed(b2Contact* contact, PedPhysicsBody* pedA, PedPhysicsBody* pedB) const;
    bool HasCollisionCarVsCar(b2Contact* contact, CarPhysicsBody* carA, CarPhysicsBody* carB) const;
    bool HasCollisionPedVsMap(int mapx, int mapy, float height) const;
    bool HasCollisionCarVsMap(b2Contact* contact, b2Fixture* fixtureCar, int mapx, int mapy) const;
    bool HasCollisionPedVsCar(b2Contact* contact, PedPhysicsBody* ped, CarPhysicsBody* car) const;

    bool ProcessProjectileVsMap(b2Contact* contact, ProjectilePhysicsBody* projectile, int mapx, int mapy) const;
    bool ProcessProjectileVsCar(b2Contact* contact, ProjectilePhysicsBody* projectile, CarPhysicsBody* car) const;
    bool ProcessProjectileVsPed(b2Contact* contact, ProjectilePhysicsBody* projectile, PedPhysicsBody* ped) const;

    // post solve collisions
    void HandleCollision(b2Contact* contact, PedPhysicsBody* ped, CarPhysicsBody* car, const b2ContactImpulse* impulse);
    void HandleCollision(b2Contact* contact, CarPhysicsBody* carA, CarPhysicsBody* carB, const b2ContactImpulse* impulse);

    // sensors
    bool ProcessSensorContact(b2Contact* contact, bool onBegin);

private:
    b2Body* mMapCollisionShape;
    b2World* mPhysicsWorld;

    float mSimulationTimeAccumulator;
    float mSimulationStepTime;

    float mGravity; // meters per second

    // bodies pools
    cxx::object_pool<PedPhysicsBody> mPedsBodiesPool;
    cxx::object_pool<CarPhysicsBody> mCarsBodiesPool;
    cxx::object_pool<ProjectilePhysicsBody> mProjectileBodiesPool;

    // bodies lists
    std::vector<PhysicsBody*> mPedsBodiesList;
    std::vector<PhysicsBody*> mCarsBodiesList;
    std::vector<PhysicsBody*> mProjectileBodiesList;
};

extern PhysicsManager gPhysics;