#pragma once

#include "PhysicsDefs.h"
#include "GameDefs.h"
#include "PhysicsDebugDraw.h"
#include "PhysicsComponents.h"

// this class manages physics and collision detections for map and objects
class PhysicsManager final: private b2ContactListener
{
public:
    PhysicsManager();

    bool Initialize();
    void Deinit();
    void UpdateFrame(Timespan deltaTime);

    // create pedestrian specific physical body
    // @param position: Coord in world
    // @param angleDegrees: Direction angle in degrees
    PedPhysicsComponent* CreatePedPhysicsComponent(const glm::vec3& position, float angleDegrees);

    // create car specific physical body
    // @param position: Coord in world
    // @param angleDegrees: Direction angle in degrees
    // @param desc: Car class description
    CarPhysicsComponent* CreateCarPhysicsComponent(const glm::vec3& position, float angleDegrees, CarStyle* desc);

    // create car wheel specific physical body
    WheelPhysicsComponent* CreateWheelPhysicsComponent();

    // free physics object
    // @param object: Object to destroy, pointer becomes invalid
    void DestroyPhysicsComponent(PedPhysicsComponent* object);
    void DestroyPhysicsComponent(CarPhysicsComponent* object);
    void DestroyPhysicsComponent(WheelPhysicsComponent* object);

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
    b2Body* mMapCollisionShape;
    b2World* mPhysicsWorld;
    float mSimulationTimeAccumulator;

    // physics components pools
    cxx::object_pool<PedPhysicsComponent> mPedsBodiesPool;
    cxx::object_pool<CarPhysicsComponent> mCarsBodiesPool;
    cxx::object_pool<WheelPhysicsComponent> mWheelsBodiesPool;
};

extern PhysicsManager gPhysics;