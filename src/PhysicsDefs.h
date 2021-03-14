#pragma once

// forwards
class PedestrianPhysics;
class CarPhysics;

// physics objects categories
enum
{
    PHYSICS_OBJCAT_MAP_SOLID_BLOCK = BIT(0), // solid map block
    PHYSICS_OBJCAT_WALL = BIT(1), // thin fence
    PHYSICS_OBJCAT_PED = BIT(2),
    PHYSICS_OBJCAT_CAR = BIT(3),
    PHYSICS_OBJCAT_OBSTACLE = BIT(4),
    PHYSICS_OBJCAT_PROJECTILE = BIT(5),
    // sensors
    PHYSICS_OBJCAT_PED_SENSOR = BIT(6),
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
    PedestrianPhysics* mPedComponent = nullptr;
    CarPhysics* mCarComponent = nullptr;

    glm::vec2 mNormal;
    glm::vec2 mIntersectionPoint;
};

// physical components query info
struct PhysicsQueryElement
{
public:
    PhysicsQueryElement() = default;
    inline void Clear()
    {
        mPedComponent = nullptr;
        mCarComponent = nullptr;
    }
public:
    // ped or car physical object
    PedestrianPhysics* mPedComponent = nullptr;
    CarPhysics* mCarComponent = nullptr;
};

// linecast result
struct PhysicsLinecastResult
{
public:
    PhysicsLinecastResult() = default;
    inline void Clear() { mHitsCount = 0; }
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
    inline void Clear() { mElementsCount = 0; }
    inline bool IsNull() const { return mElementsCount == 0; }
    inline bool IsFull() const { return mElementsCount == MaxPhysicsQueryElements; }
public:
    int mElementsCount = 0;
    PhysicsQueryElement mElements[MaxPhysicsQueryElements];
};