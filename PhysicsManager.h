#pragma once

#include "GameDefs.h"

// this class manages physics and collision detections for map and objects
class PhysicsManager final: public cxx::noncopyable
{
public:
    // works in single dimension
    bool RaycastMapWall(const glm::vec3& fromPoint, const glm::vec3& toPoint, glm::vec3& outPoint);

private:
};

extern PhysicsManager gPhysics;