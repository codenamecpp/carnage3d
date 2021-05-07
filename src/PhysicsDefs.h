#pragma once

// forwards
class PhysicsBody;
class Collider;
class Collision;

// constants
const int MaxPhysicsQueryElements = 32;
const int MaxCollisionContactPoints = 2;

enum PhysicsBodyFlags
{
    PhysicsBodyFlags_None = 0, // dynamic, handle collisions
    PhysicsBodyFlags_Static = BIT(0), // object is stationary and ignore forces
    PhysicsBodyFlags_Linked = BIT(1), // object tied to parent object
    PhysicsBodyFlags_Disabled = BIT(2), // no collisions, no gravity
    PhysicsBodyFlags_NoGravity = BIT(3), // no gravity
    PhysicsBodyFlags_FixRotation = BIT(4), // prevent rotation
    PhysicsBodyFlags_Bullet = BIT(5), // prevent tunneling effect for fast moving bodies
};
decl_enum_as_flags(PhysicsBodyFlags);

enum ColliderFlags
{
    ColliderFlags_None = 0,
};
decl_enum_as_flags(ColliderFlags);

enum CollisionGroup: unsigned short
{
    CollisionGroup_None = 0,
    CollisionGroup_MapBlock = BIT(0), // solid map block
    CollisionGroup_Wall = BIT(1), // thin fence
    CollisionGroup_Pedestrian = BIT(2),
    CollisionGroup_Car = BIT(3),
    CollisionGroup_Obstacle = BIT(4),
    CollisionGroup_Projectile = BIT(5),

    CollisionGroup_All = 0xFFFF // mask
};
decl_enum_as_flags(CollisionGroup);

// physics shape material properties
struct PhysicsMaterial
{
public:
    PhysicsMaterial() = default;
    void Clear()
    {
        mFriction = 0.0f;
        mRestitution = 0.0f;
        mDensity = 0.0f;
    }
public:
    float mFriction = 0.0f; // friction coefficient, usually in the range [0,1]
    float mRestitution = 0.0f; // restitution (elasticity) usually in the range [0,1]
    float mDensity = 0.0f; // density, usually in kg/m^2
};

enum eCollisionShapeType
{
    eCollisionShape_Null, // not specified
    eCollisionShape_Circle,
    eCollisionShape_Edge, // line segment
    eCollisionShape_Box,
};

struct CollisionShape
{
public:
    CollisionShape()
    {
    }
    // setup circle shape
    void SetAsCircle(float circleRadius)
    {
        mType = eCollisionShape_Circle;
        mCircle.mRadius = circleRadius;
    }
    // setup box shape
    void SetAsBox(const glm::vec3& halfExtents, const glm::vec3& center)
    {
        mType = eCollisionShape_Box;
        mBox.mHalfExtents = halfExtents;
        mBox.mCenter = center;
    }
    // setup edge shape
    void SetAsEdge(const glm::vec2& edgePoint0, const glm::vec2& edgePoint1)
    {
        mType = eCollisionShape_Edge;
        mEdge.mEdgePoint0 = edgePoint0;
        mEdge.mEdgePoint1 = edgePoint1;
    }
    void Clear()
    {
        mType = eCollisionShape_Null;
    }
public:
    eCollisionShapeType mType = eCollisionShape_Null;

    union
    {
        // circle shape data
        struct
        {
            float mRadius;
        } 
        mCircle;
        // edge shape data
        struct
        {
            glm::vec2 mEdgePoint0;
            glm::vec2 mEdgePoint1;
        } 
        mEdge;
        // box shape data
        struct
        {
            glm::vec3 mHalfExtents;
            glm::vec3 mCenter; // center offset in local coordinates
        } 
        mBox;
    };
};

// physical components query info
struct PhysicsQueryElement
{
public:
    PhysicsQueryElement() = default;
    inline void SetNull()
    {
        mPhysicsObject = nullptr;
    }
public:
    PhysicsBody* mPhysicsObject = nullptr;

    // these valid for linecast only:
    glm::vec2 mNormal;
    glm::vec2 mIntersectionPoint;
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

// collision contact point info
struct ContactPoint
{
public:
    ContactPoint() = default;
    ContactPoint(const glm::vec2& position, float positionY, const glm::vec2& normal, float separation)
        : mPosition(position)
        , mNormal(normal)
        , mSeparation(separation)
        , mPositionY(positionY)
    {
    }
public:
    glm::vec2 mPosition; // contact point position, world space 
    glm::vec2 mNormal; // normal of contact point, world space
    float mPositionY; // additional dimension, height, world space
    float mSeparation; // negative value indicates overlap, in meters
};