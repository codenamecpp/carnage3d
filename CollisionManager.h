#pragma once

#include "GameDefs.h"

// collision detections for map and objects
class CollisionManager final: public cxx::noncopyable
{
public:
    // works in single dimension
    bool RaycastMapWall(const glm::vec3& fromPoint, const glm::vec3& toPoint, glm::vec3& outPoint);

private:
};

extern CollisionManager gCollisionManager;