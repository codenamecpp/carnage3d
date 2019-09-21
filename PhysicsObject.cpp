#include "stdafx.h"
#include "PhysicsObject.h"

PhysicsObject::PhysicsObject()
    : mHeight()
    , mFalling()
    , mGhost()
    , mPhysicsBody()
    , mPhysicsWorld()
    , mDepth()
    , mInCollidingWithWall()
{
}

PhysicsObject::~PhysicsObject()
{
    debug_assert(mPhysicsWorld);
    debug_assert(mPhysicsBody);

    mPhysicsWorld->DestroyBody(mPhysicsBody);
}

void PhysicsObject::SetPosition(const glm::vec3& position)
{
    debug_assert(mPhysicsBody);

    mHeight = position.y;

    b2Vec2 b2position { position.x, position.z };
    mPhysicsBody->SetTransform(b2position, mPhysicsBody->GetAngle());
}

void PhysicsObject::SetPosition(const glm::vec3& position, float angleDegrees)
{
    debug_assert(mPhysicsBody);

    mHeight = position.y;

    b2Vec2 b2position { position.x, position.z };
    mPhysicsBody->SetTransform(b2position, glm::radians(angleDegrees));
}

void PhysicsObject::SetAngleDegrees(float angleDegrees)
{
    debug_assert(mPhysicsBody);

    mPhysicsBody->SetTransform(mPhysicsBody->GetPosition(), glm::radians(angleDegrees));
}

void PhysicsObject::SetAngleRadians(float angleRadians)
{
    debug_assert(mPhysicsBody);

    mPhysicsBody->SetTransform(mPhysicsBody->GetPosition(), angleRadians);
}

void PhysicsObject::ApplyForce(const glm::vec3& force)
{
    debug_assert(mPhysicsBody);

    b2Vec2 b2Force { force.x, force.z };
    mPhysicsBody->ApplyForceToCenter(b2Force, true);
}

void PhysicsObject::ApplyLinearImpulse(const glm::vec3& impulse)
{
    debug_assert(mPhysicsBody);

    b2Vec2 b2Impulse { impulse.x, impulse.z };
    mPhysicsBody->ApplyLinearImpulseToCenter(b2Impulse, true);
}

glm::vec3 PhysicsObject::GetPosition() const
{
    debug_assert(mPhysicsBody);

    const b2Vec2& b2position = mPhysicsBody->GetPosition();
    return { b2position.x, mHeight, b2position.y };
}

glm::vec3 PhysicsObject::GetLinearVelocity() const
{
    debug_assert(mPhysicsBody);

    const b2Vec2& b2position = mPhysicsBody->GetLinearVelocity();
    return { b2position.x, 0.0f, b2position.y };
}

float PhysicsObject::GetAngleDegrees() const
{
    debug_assert(mPhysicsBody);

    float angleDegrees = glm::degrees(mPhysicsBody->GetAngle());
    return cxx::normalize_angle_180(angleDegrees);
}

float PhysicsObject::GetAngleRadians() const
{
    debug_assert(mPhysicsBody);

    return mPhysicsBody->GetAngle();
}

float PhysicsObject::GetAngularVelocity() const
{
    debug_assert(mPhysicsBody);

    float angularVelocity = glm::degrees(mPhysicsBody->GetAngularVelocity());
    return cxx::normalize_angle_180(angularVelocity);
}

void PhysicsObject::ApplyAngularImpulse(float impulse)
{
    debug_assert(mPhysicsBody);

    mPhysicsBody->ApplyAngularImpulse(impulse, true);
}

void PhysicsObject::SetAngularVelocity(float angularVelocity)
{
    debug_assert(mPhysicsBody);

    mPhysicsBody->SetAngularVelocity(glm::radians(angularVelocity));
}

void PhysicsObject::SetLinearVelocity(const glm::vec3& velocity)
{
    debug_assert(mPhysicsBody);

    b2Vec2 b2vec { velocity.x, velocity.z };
    mPhysicsBody->SetLinearVelocity(b2vec);
}
