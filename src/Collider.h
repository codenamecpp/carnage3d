#pragma once

#include "PhysicsDefs.h"

class Collider final: public cxx::noncopyable
{
    friend class PhysicsManager;
    friend class PhysicsBody;

public:
    // readonly
    GameObject* mGameObject; // owner game object
    PhysicsBody* mPhysicsBody; // owner physics body
    CollisionShape mShapeData;

public:
    Collider(PhysicsBody* physicsBody, const CollisionShape& shapeData, const PhysicsMaterial& shapeMaterial,
        CollisionGroup shapeCollisionGroup,
        CollisionGroup shapeCollidesWith, ColliderFlags shapeFlags);

    // Propertry setters
    void SetShapeMaterial(const PhysicsMaterial& shapeMaterial);
    void SetShapeCollisionGroup(CollisionGroup collisionGroup, CollisionGroup collidesWith);

    // Properties getters
    void GetShapeMaterial(PhysicsMaterial& shapeMaterial) const;
    void GetShapeCollisionGroup(CollisionGroup& collisionGroup, CollisionGroup& collidesWith) const;

    // Shape flags
    void SetShapeCollisionFlags(ColliderFlags flags);
    void ChangeShapeCollisionFlags(ColliderFlags enableFlags, ColliderFlags disableFlags);
    void GetShapeCollisionFlags(ColliderFlags& flags) const;

private:
    void DestroyFixture();

private:
    b2Fixture* mBox2Fixture = nullptr;
};