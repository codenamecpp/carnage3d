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

    // create level map body, used internally
    PhysicsObject* CreateMapBody();

    // free physics object
    // @param object: Object to destroy, pointer becomes invalid
    void DestroyPhysicsObject(PhysicsObject* object);

    // get real height at specified map point
    // @param position: Current position on map
    float GetHeightAtPosition(const glm::vec3& position) const;

private:
    // apply gravity forces and correct z coord for pedestrians
    void UpdatePedsGravity(Timespan deltaTime);

    // override b2ContactFilter
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

private:
    PhysicsDebugDraw mDebugDraw;
    b2World* mPhysicsWorld;
    PhysicsObject* mMapPhysicsBody;
    float mSimulationTimeAccumulator;
    cxx::object_pool<PhysicsObject> mObjectsPool;
};

extern PhysicsManager gPhysics;