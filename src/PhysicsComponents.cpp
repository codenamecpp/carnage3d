#include "stdafx.h"
#include "PhysicsComponents.h"
#include "PhysicsDefs.h"
#include "Pedestrian.h"

PhysicsComponent::PhysicsComponent(b2World* physicsWorld)
    : mHeight()
    , mOnTheGround()
    , mPhysicsWorld(physicsWorld)
    , mPhysicsBody()
{
    debug_assert(physicsWorld);
}

void PhysicsComponent::SetPosition(const glm::vec3& position)
{
    mHeight = position.y;

    b2Vec2 b2position { position.x * PHYSICS_SCALE, position.z * PHYSICS_SCALE };
    mPhysicsBody->SetTransform(b2position, mPhysicsBody->GetAngle());
}

void PhysicsComponent::SetPosition(const glm::vec3& position, cxx::angle_t rotationAngle)
{
    mHeight = position.y;

    b2Vec2 b2position { position.x * PHYSICS_SCALE, position.z * PHYSICS_SCALE };
    mPhysicsBody->SetTransform(b2position, rotationAngle.to_radians());
}

void PhysicsComponent::SetRotationAngle(cxx::angle_t rotationAngle)
{
    mPhysicsBody->SetTransform(mPhysicsBody->GetPosition(), rotationAngle.to_radians());
}

cxx::angle_t PhysicsComponent::GetRotationAngle() const
{
    cxx::angle_t rotationAngle = cxx::angle_t::from_radians(mPhysicsBody->GetAngle());
    rotationAngle.normalize_angle_180();
    return rotationAngle;
}

void PhysicsComponent::AddForce(const glm::vec2& force)
{
    b2Vec2 b2Force { force.x * PHYSICS_SCALE, force.y * PHYSICS_SCALE };
    mPhysicsBody->ApplyForceToCenter(b2Force, true);
}

void PhysicsComponent::AddLinearImpulse(const glm::vec2& impulse)
{
    b2Vec2 b2Impulse { impulse.x * PHYSICS_SCALE, impulse.y * PHYSICS_SCALE };
    mPhysicsBody->ApplyLinearImpulseToCenter(b2Impulse, true);
}

glm::vec3 PhysicsComponent::GetPosition() const
{
    const b2Vec2& b2position = mPhysicsBody->GetPosition();
    return { b2position.x / PHYSICS_SCALE, mHeight, b2position.y / PHYSICS_SCALE };
}

glm::vec2 PhysicsComponent::GetLinearVelocity() const
{
    const b2Vec2& b2position = mPhysicsBody->GetLinearVelocity();
    return { b2position.x / PHYSICS_SCALE, b2position.y / PHYSICS_SCALE };
}

float PhysicsComponent::GetAngularVelocity() const
{
    float angularVelocity = glm::degrees(mPhysicsBody->GetAngularVelocity());
    return angularVelocity;
}

void PhysicsComponent::AddAngularImpulse(float impulse)
{
    mPhysicsBody->ApplyAngularImpulse(impulse, true);
}

void PhysicsComponent::SetAngularVelocity(float angularVelocity)
{
    mPhysicsBody->SetAngularVelocity(glm::radians(angularVelocity));
}

void PhysicsComponent::SetLinearVelocity(const glm::vec2& velocity)
{
    b2Vec2 b2vec { velocity.x * PHYSICS_SCALE, velocity.y * PHYSICS_SCALE };
    mPhysicsBody->SetLinearVelocity(b2vec);
}

void PhysicsComponent::ClearForces()
{
    b2Vec2 nullVector { 0.0f, 0.0f };
    mPhysicsBody->SetLinearVelocity(nullVector);
    mPhysicsBody->SetAngularVelocity(0.0f);
}

glm::vec2 PhysicsComponent::GetSignVector() const
{
    float angleRadians = mPhysicsBody->GetAngle();
    glm::vec2 signVector 
    {
        cos(angleRadians), sin(angleRadians)
    };
    return signVector;
}

//////////////////////////////////////////////////////////////////////////

PedPhysicsComponent::PedPhysicsComponent(b2World* physicsWorld)
    : PhysicsComponent(physicsWorld)
{
    // create body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.fixedRotation = true;
    bodyDef.userData = this;

    mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(mPhysicsBody);
    
    b2CircleShape shapeDef;
    shapeDef.m_radius = PHYSICS_PED_BOUNDING_SPHERE_RADIUS * PHYSICS_SCALE;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 0.3f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_PED;

    b2Fixture* b2fixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);

    // create sensor
    shapeDef.m_radius = PHYSICS_PED_SENSOR_SPHERE_RADIUS * PHYSICS_SCALE;
    fixtureDef.shape = &shapeDef;
    fixtureDef.isSensor = true;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_PED_SENSOR;
    fixtureDef.filter.maskBits = PHYSICS_OBJCAT_PED | PHYSICS_OBJCAT_CAR;

    b2Fixture* b2sensorFixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);
}

PedPhysicsComponent::~PedPhysicsComponent()
{
}

void PedPhysicsComponent::UpdateFrame(Timespan deltaTime)
{

}

void PedPhysicsComponent::SetFalling(bool isFalling)
{
    if (isFalling == mFalling)
        return;

    mFalling = isFalling;
    if (mFalling)
    {
        b2Vec2 velocity = mPhysicsBody->GetLinearVelocity();
        ClearForces();
        velocity.Normalize();
        mPhysicsBody->SetLinearVelocity(velocity);
    }
}

void PedPhysicsComponent::HandleCarContactBegin()
{
    ++mContactingCars;
}

void PedPhysicsComponent::HandleCarContactEnd()
{
    --mContactingCars;

    if (mContactingCars < 0)
    {
        debug_assert(false);
        mContactingCars = 0;
    }
}

bool PedPhysicsComponent::ShouldCollideWith(unsigned int bits) const
{
    debug_assert(bits);

    ePedestrianState currState = mReferencePed->mCurrentStateID;
    if (currState == ePedestrianState_Falling || currState == ePedestrianState_SlideOnCar)
    {
        return (bits & (PHYSICS_OBJCAT_MAP_SOLID_BLOCK | PHYSICS_OBJCAT_WALL)) > 0;
    }

    if (currState == ePedestrianState_EnteringCar || currState == ePedestrianState_ExitingCar || 
        currState == ePedestrianState_DrivingCar)
    {
        return false;
    }

    if (currState == ePedestrianState_Dying || currState == ePedestrianState_Dead)
    {
        return false;
    }

    if (currState == ePedestrianState_KnockedDown)
    {
        return (bits & PHYSICS_OBJCAT_PED) == 0; // collide to all except for other peds
    }

    return true;    
}

//////////////////////////////////////////////////////////////////////////

CarPhysicsComponent::CarPhysicsComponent(b2World* physicsWorld, CarStyle* desc)
    : PhysicsComponent(physicsWorld)
{

    // create body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.userData = this;
    bodyDef.linearDamping = 0.15f;
    bodyDef.angularDamping = 0.3f;

    mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(mPhysicsBody);
    //physicsObject->mDepth = (1.0f * desc->mDepth) / MAP_BLOCK_TEXTURE_DIMS;
    
    b2PolygonShape shapeDef;
    shapeDef.SetAsBox(((1.0f * desc->mHeight) / MAP_BLOCK_TEXTURE_DIMS) * 0.5f * PHYSICS_SCALE, 
        ((1.0f * desc->mWidth) / MAP_BLOCK_TEXTURE_DIMS) * 0.5f * PHYSICS_SCALE);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 700.0f;
    fixtureDef.friction = 0.1f;
    fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_CAR;

    b2Fixture* b2fixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);
}

CarPhysicsComponent::~CarPhysicsComponent()
{
}

void CarPhysicsComponent::UpdateFrame(Timespan deltaTime)
{

}

//////////////////////////////////////////////////////////////////////////

WheelPhysicsComponent::WheelPhysicsComponent(b2World* physicsWorld)
    : PhysicsComponent(physicsWorld)
{
}

WheelPhysicsComponent::~WheelPhysicsComponent()
{
}

void WheelPhysicsComponent::UpdateFrame(Timespan deltaTime)
{

}
