#pragma once

// forwards
class PedPhysicsComponent;
class CarPhysicsComponent;

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

const int MaxPhysicsQueryElements = 32;

// linecast hit info
struct PhysicsLinecastHit
{
public:
    PhysicsLinecastHit() = default;
    inline void SetNull()
    {
        mPedComponent = nullptr;
        mCarComponent = nullptr;
    }
public:
    // ped or car physical object
    PedPhysicsComponent* mPedComponent = nullptr;
    CarPhysicsComponent* mCarComponent = nullptr;

    glm::vec2 mNormal;
    glm::vec2 mIntersectionPoint;
};

// physical components query info
struct PhysicsQueryElement
{
public:
    PhysicsQueryElement() = default;
    inline void SetNull()
    {
        mPedComponent = nullptr;
        mCarComponent = nullptr;
    }
public:
    // ped or car physical object
    PedPhysicsComponent* mPedComponent = nullptr;
    CarPhysicsComponent* mCarComponent = nullptr;
};

// linecast result
struct PhysicsLinecastResult
{
public:
    PhysicsLinecastResult() = default;
    inline void SetNull() { mHitsCount = 0; }
    inline bool IsNull() const { return mHitsCount == 0; }
    inline bool IsFull() const { return mHitsCount == MaxPhysicsQueryElements; }
public:
    int mHitsCount = 0;
    PhysicsLinecastHit mHits[MaxPhysicsQueryElements];
};

// physical components query result
struct PhysicsQueryResult
{
public:
    PhysicsQueryResult() = default;
    inline void SetNull() { mElementsCount = 0; }
    inline bool IsNull() const { return mElementsCount == 0; }
    inline bool IsFull() const { return mElementsCount == MaxPhysicsQueryElements; }
public:
    int mElementsCount = 0;
    PhysicsQueryElement mElements[MaxPhysicsQueryElements];
};