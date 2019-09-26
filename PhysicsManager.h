#pragma once

#include "PhysicsDefs.h"
#include "GameDefs.h"
#include "PhysicsDebugDraw.h"
#include "PhysicsObject.h"

// this class manages physics and collision detections for map and objects
class PhysicsManager final: public cxx::noncopyable
    , private b2ContactListener
{
public:
    PhysicsManager();

    bool Initialize();
    void Deinit();
    void UpdateFrame(Timespan deltaTime);

    // create pedestrian specific physical body
    // @param position: Coord in world
    // @param angleDegrees: Direction angle in degrees
    PhysicsObject* CreatePedestrianBody(const glm::vec3& position, float angleDegrees);

    // create vehicle specific physical body
    // @param position: Coord in world
    // @param angleDegrees: Direction angle in degrees
    // @param desc: Car class description
    PhysicsObject* CreateVehicleBody(const glm::vec3& position, float angleDegrees, CarStyle* desc);

    // free physics object
    // @param object: Object to destroy, pointer becomes invalid
    void DestroyPhysicsObject(PhysicsObject* object);

private:
    // create level map body, used internally
    void CreateMapCollisionShape();

    // apply gravity forces and correct y coord for objects
    void FixedStepPedsGravity();

    // override b2ContactFilter
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

    bool HasCollisionPedestrianVsMap(int mapx, int mapz, float height) const;
    bool HasCollisionPedestrianVsCar(b2Contact* contact, b2Fixture* pedestrianFixture, b2Fixture* carFixture);

private:
    PhysicsDebugDraw mDebugDraw;
    b2World* mPhysicsWorld;
    PhysicsObject* mMapCollisionBody;
    float mSimulationTimeAccumulator;

    cxx::object_pool<PhysicsObject> mObjectsPool;
};

extern PhysicsManager gPhysics;