#pragma once

// forwards
class PhysicsObject;

#define PHYSICS_PED_BOUNDING_SPHERE_RADIUS 0.10f
#define PHYSICS_SIMULATION_STEP (1.0f / 60.0f)
#define PHYSICS_GRAVITY (9.8f)
#define PHYSICS_SCALE 10.0f

// physics objects categories
enum
{
    PHYSICS_OBJCAT_MAP_SOLID_BLOCK = (1 << 0), // solid map block
    PHYSICS_OBJCAT_WALL = (1 << 1), // thin fence
    PHYSICS_OBJCAT_PED = (1 << 2),
    PHYSICS_OBJCAT_CAR = (1 << 3),
    PHYSICS_OBJCAT_MAPOBJECT = (1 << 4),
};

const int MaxPhysicsQueryItems = 64;

struct PhysicsQueryResult
{
public:
    PhysicsObject* mObjects[MaxPhysicsQueryItems];
    int mNumQueryItems = 0; // real number of items in objects list
};