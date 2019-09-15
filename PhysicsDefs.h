#pragma once

// forwards
class PhysicsObject;

#define PHYSICS_PED_BOUNDING_SPHERE_RADIUS 0.10f

// physics objects categories
enum
{
    PHYSICS_OBJCAT_MAP = (1 << 0),
    PHYSICS_OBJCAT_PED = (1 << 1),
    PHYSICS_OBJCAT_CAR = (1 << 2),
    PHYSICS_OBJCAT_MAPOBJECT = (1 << 3),
};