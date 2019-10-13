#pragma once

// forwards
class PedPhysicsComponent;
class CarPhysicsComponent;
class WheelPhysicsComponent;

#define PHYSICS_PED_BOUNDING_SPHERE_RADIUS 0.10f
#define PHYSICS_PED_SENSOR_SPHERE_RADIUS (PHYSICS_PED_BOUNDING_SPHERE_RADIUS)
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

    // sensors
    PHYSICS_OBJCAT_PED_SENSOR = (1 << 5),
};

// holds physics world query result
const int MaxPhysicsQueryElements = 16;
struct PhysicsQueryResult
{
public:
    PhysicsQueryResult() = default;

    inline bool AddElement(PedPhysicsComponent* component)
    {
        debug_assert(component);
        if (mPedsCount < MaxPhysicsQueryElements)
        {
            mPedsList[mPedsCount++] = component;
            return true;
        }
        return false;
    }

    inline bool AddElement(CarPhysicsComponent* component)
    {
        debug_assert(component);
        if (mCarsCount < MaxPhysicsQueryElements)
        {
            mCarsList[mCarsCount++] = component;
            return true;
        }
        return false;
    }

    inline void SetNull()
    {
        mPedsCount = 0;
        mCarsCount = 0;
    }

    inline bool IsNull() const
    {
        return mPedsCount == 0 && mCarsCount == 0;
    }

public:
    int mPedsCount = 0;
    int mCarsCount = 0;
    PedPhysicsComponent* mPedsList[MaxPhysicsQueryElements];
    CarPhysicsComponent* mCarsList[MaxPhysicsQueryElements];
};