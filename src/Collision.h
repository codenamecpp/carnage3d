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
    // Get total impulse applied to collider pair to resolve collision
    // Sometimes it is 0, for example, when multiple objects are spawned in one place
    inline float GetContactImpulse() const
    {
        return mContactImpulse;
    }
private:
    void SetupWithBox2Data(b2Contact* contact, b2Fixture* thisFixture, b2Fixture* thatFixture, const b2ContactImpulse* contactImpulse);

private:
    float mContactImpulse = 0.0f;
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
    // Get total impulse applied to collider to resolve collision
    // Sometimes it is 0, for example, when object spawned in wall
    inline float GetContactImpulse() const
    {
        return mContactImpulse;
    }
private:
    void SetupWithBox2Data(b2Contact* contact, b2Fixture* objectFixture, const MapBlockInfo* mapBlockInfo, const b2ContactImpulse* contactImpulse);

private:
    float mContactImpulse = 0.0f;
};