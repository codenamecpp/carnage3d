#pragma once

#include "GameDefs.h"
#include "PhysicsDebugDraw.h"

// this class manages physics and collision detections for map and objects
class PhysicsManager final: public cxx::noncopyable
{
public:
    PhysicsManager();
    bool Initialize();
    void Deinit();
    void UpdateFrame(Timespan deltaTime);

    void EnableDebugDraw(bool isEnabled);

private:
    PhysicsDebugDraw mDebugDraw;
    b2World* mPhysicsWorld;
    Timespan mTimeSinceLastSimulation;
};

extern PhysicsManager gPhysics;