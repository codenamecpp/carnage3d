#include "stdafx.h"
#include "PhysicsBody.h"
#include "PhysicsDefs.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "Projectile.h"
#include "Explosion.h"
#include "PhysicsManager.h"
#include "TimeManager.h"
#include "Box2D_Helpers.h"
#include "CarnageGame.h"
#include "Collider.h"

//////////////////////////////////////////////////////////////////////////

using CollidersPool = cxx::object_pool<Collider>;
static CollidersPool gCollidersPool;

//////////////////////////////////////////////////////////////////////////

PhysicsBody::PhysicsBody(GameObject* owner, PhysicsBodyFlags flags)
    : mPositionY()
    , mBox2Body()
    , mBodyFlags(flags)
    , mGameObject(owner)
{
    debug_assert(mGameObject);

    // create body
    bool isStatic = CheckFlags(PhysicsBodyFlags_Static);
    bool isLinked = CheckFlags(PhysicsBodyFlags_Linked);

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody; // default

    if (isLinked)
    {
        bodyDef.type = b2_kinematicBody;
    }
    else if (isStatic)
    {
        bodyDef.type = b2_staticBody;
    }

    bodyDef.fixedRotation = CheckFlags(PhysicsBodyFlags_FixRotation);
    bodyDef.bullet = CheckFlags(PhysicsBodyFlags_Bullet);
    bodyDef.active = CheckFlags(PhysicsBodyFlags_Disabled) == false;
    bodyDef.userData = this;
    // setup initial transform
    if (mGameObject)
    {
        bodyDef.position = convert_vec2(mGameObject->mTransform.GetPosition2());
        bodyDef.angle = mGameObject->mTransform.mOrientation.to_radians();
    }
    b2World* b2PhysicsWorld = gPhysics.mBox2World;
    debug_assert(b2PhysicsWorld);
    mBox2Body = b2PhysicsWorld->CreateBody(&bodyDef);
    debug_assert(mBox2Body);
}

PhysicsBody::~PhysicsBody()
{
    DeleteColliders();

    if (mBox2Body)
    {
        b2World* b2PhysicsWorld = gPhysics.mBox2World;
        debug_assert(b2PhysicsWorld);

        b2PhysicsWorld->DestroyBody(mBox2Body);
        mBox2Body = nullptr;
    }
}

void PhysicsBody::SetupFlags(PhysicsBodyFlags flags)
{
    if (mBodyFlags == flags) // nothing to change
        return;

    mBodyFlags = flags;

    bool isStatic = CheckFlags(PhysicsBodyFlags_Static);
    bool isLinked = CheckFlags(PhysicsBodyFlags_Linked);

    b2BodyType newBodyType = b2_dynamicBody;
    if (isLinked)
    {
        newBodyType = b2_kinematicBody;
    }
    else if (isStatic)
    {
        newBodyType = b2_staticBody;
    }

    if (newBodyType != mBox2Body->GetType())
    {
        mBox2Body->SetType(newBodyType);
    }

    bool isDisabled = CheckFlags(PhysicsBodyFlags_Disabled);
    mBox2Body->SetActive(!isDisabled);

    bool isHovering = CheckFlags(PhysicsBodyFlags_NoGravity);

    if (mFalling)
    {
        // todo: cancel current fall
    }

    bool isFixRotation = CheckFlags(PhysicsBodyFlags_FixRotation);
    mBox2Body->SetFixedRotation(isFixRotation);

    bool isBullet = CheckFlags(PhysicsBodyFlags_Bullet);
    mBox2Body->SetBullet(isBullet);
}

void PhysicsBody::ChangeFlags(PhysicsBodyFlags enableFlags, PhysicsBodyFlags disableFlags)
{
    PhysicsBodyFlags newFlags = (mBodyFlags | enableFlags) & ~disableFlags;
    SetupFlags(newFlags);
}

bool PhysicsBody::CheckFlags(PhysicsBodyFlags flags) const
{
    return (mBodyFlags & flags) > 0;
}

Collider* PhysicsBody::AddCollider(int colliderIndex, const CollisionShape& shapeData, const PhysicsMaterial& shapeMaterial,
    CollisionGroup collisionGroup, 
    CollisionGroup collidesWith, ColliderFlags colliderFlags)
{
    debug_assert(!gPhysics.IsSimulationStepInProgress());
    if (colliderIndex < 0)
    {
        debug_assert(false);
        return nullptr;
    }

    if (colliderIndex >= (int) mColliders.size())
    {
        mColliders.resize(colliderIndex + 1);
    }

    if (mColliders[colliderIndex] == nullptr)
    {
        mColliders[colliderIndex] = gCollidersPool.create(this, shapeData, shapeMaterial, collisionGroup, collidesWith, colliderFlags);
        debug_assert(mColliders[colliderIndex]);
    }
    else
    {
        debug_assert(false); // shape exists!
    }
    return mColliders[colliderIndex];
}

bool PhysicsBody::DeleteCollider(int colliderIndex)
{
    debug_assert(!gPhysics.IsSimulationStepInProgress());
    Collider* collider = GetColliderWithIndex(colliderIndex);
    if (collider == nullptr)
        return false;

    collider->DestroyFixture();
    mColliders[colliderIndex] = nullptr;

    gCollidersPool.destroy(collider);
    return true;
}

bool PhysicsBody::DeleteCollider(Collider* collider)
{
    debug_assert(collider);
    debug_assert(!gPhysics.IsSimulationStepInProgress());
    int colliderIndex = 0;
    for (Collider* currCollider: mColliders)
    {
        if (currCollider == collider)
        {
            currCollider->DestroyFixture();
            mColliders[colliderIndex] = nullptr;

            gCollidersPool.destroy(currCollider);
            return true;
        }
        ++colliderIndex;
    }
    debug_assert(false);
    return false;
}

void PhysicsBody::DeleteColliders()
{
    debug_assert(!gPhysics.IsSimulationStepInProgress());
    int colliderIndex = 0;
    for (Collider* currCollider: mColliders)
    {
        currCollider->DestroyFixture();
        gCollidersPool.destroy(currCollider);
    }
    mColliders.clear();
}

Collider* PhysicsBody::GetColliderWithIndex(int colliderIndex) const
{
    if (colliderIndex < 0)
    {
        debug_assert(false);
        return nullptr;
    }

    if (colliderIndex < (int) mColliders.size())
        return mColliders[colliderIndex];

    return nullptr;
}

void PhysicsBody::SetTransform(const glm::vec3& position)
{
    mPositionY = position.y;

    b2Vec2 b2position { position.x, position.z };
    mBox2Body->SetTransform(b2position, mBox2Body->GetAngle());
}

void PhysicsBody::SetTransform(const glm::vec3& position, cxx::angle_t rotationAngle)
{
    mPositionY = position.y;

    b2Vec2 b2position { position.x, position.z };
    mBox2Body->SetTransform(b2position, rotationAngle.to_radians());
}

void PhysicsBody::SetOrientation(cxx::angle_t rotationAngle)
{
    mBox2Body->SetTransform(mBox2Body->GetPosition(), rotationAngle.to_radians());
}

cxx::angle_t PhysicsBody::GetOrientation() const
{
    cxx::angle_t rotationAngle = cxx::angle_t::from_radians(mBox2Body->GetAngle());
    return rotationAngle;
}

void PhysicsBody::SetSignVector(const glm::vec2& signDirection)
{
    float rotationAngleRadians = atan2f(signDirection.y, signDirection.x);
    mBox2Body->SetTransform(mBox2Body->GetPosition(), rotationAngleRadians);
}

void PhysicsBody::AddForce(const glm::vec2& force)
{
    b2Vec2 b2Force = convert_vec2(force);
    mBox2Body->ApplyForceToCenter(b2Force, true);
}

void PhysicsBody::AddForce(const glm::vec2& force, const glm::vec2& position)
{
    if (glm::length2(force) > 0.0f)
    {
        b2Vec2 b2Force = convert_vec2(force);
        b2Vec2 b2Position = convert_vec2(position);

        mBox2Body->ApplyForce(b2Force, b2Position, true);
    }
}

void PhysicsBody::ApplyLinearImpulse(const glm::vec2& impulse)
{
    if (glm::length2(impulse) > 0.0f)
    {
        b2Vec2 b2Impulse = convert_vec2(impulse);
        mBox2Body->ApplyLinearImpulseToCenter(b2Impulse, true);
    }
}

void PhysicsBody::ApplyLinearImpulse(const glm::vec2& impulse, const glm::vec2& position)
{
    if (glm::length2(impulse) > 0.0f)
    {
        b2Vec2 b2Impulse = convert_vec2(impulse);
        b2Vec2 b2Position = convert_vec2(position);

        mBox2Body->ApplyLinearImpulse(b2Impulse, b2Position, true);
    }
}

glm::vec3 PhysicsBody::GetPosition() const
{
    const b2Vec2& b2position = mBox2Body->GetPosition();
    return { b2position.x, mPositionY, b2position.y };
}

glm::vec2 PhysicsBody::GetPosition2() const
{
    const b2Vec2& b2position = mBox2Body->GetPosition();
    return convert_vec2(b2position);
}

glm::vec2 PhysicsBody::GetLinearVelocity() const
{
    const b2Vec2& b2velocity = mBox2Body->GetLinearVelocity();
    return convert_vec2(b2velocity);
}

glm::vec2 PhysicsBody::GetLinearVelocityFromWorldPoint(const glm::vec2& worldPosition) const
{
    b2Vec2 b2point = convert_vec2(worldPosition);
    b2Vec2 b2vec = mBox2Body->GetLinearVelocityFromWorldPoint(b2point);
    return convert_vec2(b2vec);
}

glm::vec2 PhysicsBody::GetLinearVelocityFromLocalPoint(const glm::vec2& localPosition) const
{
    b2Vec2 b2point = convert_vec2(localPosition);
    b2Vec2 b2vec = mBox2Body->GetLinearVelocityFromLocalPoint(b2point);
    return convert_vec2(b2vec);
}

cxx::angle_t PhysicsBody::GetAngularVelocity() const
{
    cxx::angle_t angularVelocity = cxx::angle_t::from_radians(mBox2Body->GetAngularVelocity());
    return angularVelocity;
}

void PhysicsBody::ApplyAngularImpulse(float impulse)
{
    if (cxx::equals_zero(impulse))
        return;
        
    mBox2Body->ApplyAngularImpulse(impulse, true);
}

void PhysicsBody::SetAngularVelocity(cxx::angle_t angularVelocity)
{
    mBox2Body->SetAngularVelocity(angularVelocity.to_radians());
}

void PhysicsBody::SetLinearVelocity(const glm::vec2& velocity)
{
    b2Vec2 b2vec = convert_vec2(velocity);
    mBox2Body->SetLinearVelocity(b2vec);
}

void PhysicsBody::ClearForces()
{
    mBox2Body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    mBox2Body->SetAngularVelocity(0.0f);
}

glm::vec2 PhysicsBody::GetSignVector() const
{
    float angleRadians = mBox2Body->GetAngle();
    return { 
        cos(angleRadians), 
        sin(angleRadians)
    };
}

glm::vec2 PhysicsBody::GetWorldPoint(const glm::vec2& localPosition) const
{
    b2Vec2 b2LocalPosition = convert_vec2(localPosition);
    b2Vec2 b2WorldPosition = mBox2Body->GetWorldPoint(b2LocalPosition);

    return convert_vec2(b2WorldPosition);
}

glm::vec2 PhysicsBody::GetLocalPoint(const glm::vec2& worldPosition) const
{
    b2Vec2 b2WorldPosition = convert_vec2(worldPosition);
    b2Vec2 b2LocalPosition = mBox2Body->GetLocalPoint(b2WorldPosition);

    return convert_vec2(b2LocalPosition);
}

glm::vec2 PhysicsBody::GetWorldVector(const glm::vec2& localVector) const
{
    b2Vec2 b2LocalVector = convert_vec2(localVector);
    b2Vec2 b2WorldVector = mBox2Body->GetWorldVector(b2LocalVector);

    return convert_vec2(b2WorldVector);
}

glm::vec2 PhysicsBody::GetLocalVector(const glm::vec2& worldVector) const
{
    b2Vec2 b2WorldVector = convert_vec2(worldVector);
    b2Vec2 b2LocalVector = mBox2Body->GetLocalVector(b2WorldVector);

    return convert_vec2(b2LocalVector);
}

bool PhysicsBody::HasColliderWithIndex(int shapeIndex) const
{
    Collider* collisionShape = GetColliderWithIndex(shapeIndex);
    return collisionShape != nullptr;
}

bool PhysicsBody::IsAwake() const
{
    return mBox2Body->IsAwake();
}

void PhysicsBody::SetAwake(bool isAwake)
{
    mBox2Body->SetAwake(isAwake);
}

float PhysicsBody::GetMass() const
{
    return mBox2Body->GetMass();
}
