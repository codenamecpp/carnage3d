#pragma once

#include "PhysicsDefs.h"

//////////////////////////////////////////////////////////////////////////

// contact between objects colliders
class Contact
{
    friend class PhysicsManager;
    friend class Collision;

public:
    // readonly
    Collider* mThisCollider = nullptr;
    Collider* mThatCollider = nullptr;

    GameObject* mThisObject = nullptr;
    GameObject* mThatObject = nullptr;

    ContactPoint mContactPoints[MaxCollisionContactPoints];
    int mContactPointsCount = 0;

public:
    Contact() = default;

    bool GetRelativeVelocity(int contactPointIndex, glm::vec2& outputVelocity) const;
    bool GetRelativeVelocity(glm::vec2& outputVelocity) const;
    bool HasContactPoints() const;

private:
    void SetupWithBox2Data(b2Contact* contact, b2Fixture* thisFixture, b2Fixture* thatFixture);
};

//////////////////////////////////////////////////////////////////////////

// collision between objects colliders
class Collision
{
    friend class PhysicsManager;

public:
    // readonly
    Contact mContactInfo;
public:
    Collision() = default;
    // Get speed at which colliders move to each other before collision, meters per second
    inline float GetImpactSpeed() const
    {
        return mImpactSpeed;
    }
private:
    void SetupWithBox2Data(b2Contact* contact, b2Fixture* thisFixture, b2Fixture* thatFixture, float impactSpeed);

private:
    float mImpactSpeed = 0.0f;
};

//////////////////////////////////////////////////////////////////////////

// collision between map and object collider
class MapCollision
{
    friend class PhysicsManager;

public:
    // readonly
    Collider* mThisCollider = nullptr;
    GameObject* mThisObject = nullptr;
    const MapBlockInfo* mMapBlockInfo = nullptr;

    ContactPoint mContactPoints[MaxCollisionContactPoints];
    int mContactPointsCount = 0;

public:
    MapCollision() = default;

    bool HasContactPoints() const;
    // Get speed at which colliders move to each other before collision, meters per second
    inline float GetImpactSpeed() const
    {
        return mImpactSpeed;
    }
private:
    void SetupWithBox2Data(b2Contact* contact, b2Fixture* objectFixture, const MapBlockInfo* mapBlockInfo, float impactSpeed);

private:
    float mImpactSpeed = 0.0f;
};