#pragma once

#include "PhysicsDefs.h"
#include "GameDefs.h"
#include "PhysicsDebugDraw.h"
#include "PhysicsObject.h"

// this class manages physics and collision detections for map and objects
class PhysicsManager final: public cxx::noncopyable
    , private b2ContactFilter
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

    // free physics object
    // @param object: Object to destroy, pointer becomes invalid
    void DestroyPhysicsObject(PhysicsObject* object);

    // show or hide debug draw information on screen
    // param isEnabled: Flag
    void EnableDebugDraw(bool isEnabled);

private:
    // create level map body, used internally
    PhysicsObject* CreateMapBody();

    // override b2ContactFilter
	bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override; 

private:
    PhysicsDebugDraw mDebugDraw;
    b2World* mPhysicsWorld;
    PhysicsObject* mMapPhysicsBody;
    float mSimulationTimeAccumulator;
    cxx::object_pool<PhysicsObject> mObjectsPool;
};

extern PhysicsManager gPhysics;