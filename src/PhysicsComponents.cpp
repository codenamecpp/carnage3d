#include "stdafx.h"
#include "PhysicsComponents.h"
#include "PhysicsDefs.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "Projectile.h"
#include "Explosion.h"
#include "PhysicsManager.h"
#include "TimeManager.h"
#include "Box2D_Helpers.h"

PhysicsBody::PhysicsBody(b2World* physicsWorld)
    : mHeight()
    , mPhysicsWorld(physicsWorld)
    , mPhysicsBody()
    , mPreviousPosition()
    , mSmoothPosition()
{
    debug_assert(physicsWorld);
}

PhysicsBody::~PhysicsBody()
{
    if (mPhysicsBody)
    {
        mPhysicsWorld->DestroyBody(mPhysicsBody);
        mPhysicsBody = nullptr;
    }
}

void PhysicsBody::SetPosition(const glm::vec3& position)
{
    mHeight = position.y;
    mPreviousPosition = position;
    mSmoothPosition = position;

    box2d::vec2 b2position { position.x, position.z };
    mPhysicsBody->SetTransform(b2position, mPhysicsBody->GetAngle());
}

void PhysicsBody::SetPosition(const glm::vec3& position, cxx::angle_t rotationAngle)
{
    mHeight = position.y;
    mPreviousPosition = position;
    mSmoothPosition = position;

    box2d::vec2 b2position { position.x, position.z };
    mPhysicsBody->SetTransform(b2position, rotationAngle.to_radians());
}

void PhysicsBody::SetRotationAngle(cxx::angle_t rotationAngle)
{
    mPhysicsBody->SetTransform(mPhysicsBody->GetPosition(), rotationAngle.to_radians());
}

cxx::angle_t PhysicsBody::GetRotationAngle() const
{
    cxx::angle_t rotationAngle = cxx::angle_t::from_radians(mPhysicsBody->GetAngle());
    rotationAngle.normalize_angle_180();
    return rotationAngle;
}

void PhysicsBody::AddForce(const glm::vec2& force)
{
    box2d::vec2 b2Force = force;
    mPhysicsBody->ApplyForceToCenter(b2Force, true);
}

void PhysicsBody::AddForce(const glm::vec2& force, const glm::vec2& position)
{
    box2d::vec2 b2Force = force;
    box2d::vec2 b2Position = position;
    mPhysicsBody->ApplyForce(b2Force, b2Position, true);
}

void PhysicsBody::AddLinearImpulse(const glm::vec2& impulse)
{
    box2d::vec2 b2Impulse = impulse;
    mPhysicsBody->ApplyLinearImpulseToCenter(b2Impulse, true);
}

void PhysicsBody::AddLinearImpulse(const glm::vec2& impulse, const glm::vec2& position)
{
    box2d::vec2 b2Impulse = impulse;
    box2d::vec2 b2Position = position;
    mPhysicsBody->ApplyLinearImpulse(b2Impulse, b2Position, true);
}

glm::vec3 PhysicsBody::GetPosition() const
{
    const b2Vec2& b2position = mPhysicsBody->GetPosition();
    return { b2position.x, mHeight, b2position.y };
}

glm::vec2 PhysicsBody::GetPosition2() const
{
    const b2Vec2& b2position = mPhysicsBody->GetPosition();
    return { b2position.x, b2position.y };
}

glm::vec2 PhysicsBody::GetLinearVelocity() const
{
    const b2Vec2& b2velocity = mPhysicsBody->GetLinearVelocity();
    return { b2velocity.x, b2velocity.y };
}

cxx::angle_t PhysicsBody::GetAngularVelocity() const
{
    cxx::angle_t angularVelocity = cxx::angle_t::from_radians(mPhysicsBody->GetAngularVelocity());
    return angularVelocity;
}

void PhysicsBody::AddAngularImpulse(float impulse)
{
    mPhysicsBody->ApplyAngularImpulse(impulse, true);
}

void PhysicsBody::SetAngularVelocity(cxx::angle_t angularVelocity)
{
    mPhysicsBody->SetAngularVelocity(angularVelocity.to_radians());
}

void PhysicsBody::SetLinearVelocity(const glm::vec2& velocity)
{
    box2d::vec2 b2vec = velocity;
    mPhysicsBody->SetLinearVelocity(b2vec);
}

void PhysicsBody::ClearForces()
{
    mPhysicsBody->SetLinearVelocity(box2d::NullVector);
    mPhysicsBody->SetAngularVelocity(0.0f);
}

glm::vec2 PhysicsBody::GetSignVector() const
{
    float angleRadians = mPhysicsBody->GetAngle();
    return { 
        cos(angleRadians), 
        sin(angleRadians)
    };
}

glm::vec2 PhysicsBody::GetWorldPoint(const glm::vec2& localPosition) const
{
    box2d::vec2 b2LocalPosition = localPosition;
    box2d::vec2 b2WorldPosition = mPhysicsBody->GetWorldPoint(b2LocalPosition);

    return b2WorldPosition;
}

glm::vec2 PhysicsBody::GetLocalPoint(const glm::vec2& worldPosition) const
{
    box2d::vec2 b2WorldPosition = worldPosition;
    box2d::vec2 b2LocalPosition = mPhysicsBody->GetLocalPoint(b2WorldPosition);

    return b2LocalPosition;
}

glm::vec2 PhysicsBody::GetWorldVector(const glm::vec2& localVector) const
{
    box2d::vec2 b2LocalVector = localVector;
    box2d::vec2 b2WorldVector = mPhysicsBody->GetWorldVector(b2LocalVector);

    return b2WorldVector;
}

glm::vec2 PhysicsBody::GetLocalVector(const glm::vec2& worldVector) const
{
    box2d::vec2 b2WorldVector = worldVector;
    box2d::vec2 b2LocalVector = mPhysicsBody->GetLocalVector(b2WorldVector);

    return b2LocalVector;
}

void PhysicsBody::SetRespawned()
{
    mFalling = false;
    mWaterContact = false;
    mFallDistance = 0.0f;
}

//////////////////////////////////////////////////////////////////////////

PedPhysicsBody::PedPhysicsBody(b2World* physicsWorld, Pedestrian* object)
    : PhysicsBody(physicsWorld)
    , mReferencePed(object)
{
    // create body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.fixedRotation = true;
    bodyDef.userData = this;

    mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(mPhysicsBody);
    
    b2CircleShape shapeDef;
    shapeDef.m_radius = gGameParams.mPedestrianBoundsSphereRadius;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 0.3f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_PED;

    b2Fixture* b2fixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);

    // create sensor
    shapeDef.m_radius = gGameParams.mPedestrianBoundsSphereRadius;
    fixtureDef.shape = &shapeDef;
    fixtureDef.isSensor = true;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_PED_SENSOR;
    fixtureDef.filter.maskBits = PHYSICS_OBJCAT_PED | PHYSICS_OBJCAT_CAR;

    b2Fixture* b2sensorFixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);
}

PedPhysicsBody::~PedPhysicsBody()
{
}

void PedPhysicsBody::SimulationStep()
{
    if (mReferencePed->mCurrentCar)
    {
        CarPhysicsBody* currentCarPhysics = mReferencePed->mCurrentCar->mPhysicsBody;

        box2d::vec2 b2LocalPosition = mCarPointLocal;
        box2d::vec2 b2WorldPosition = currentCarPhysics->mPhysicsBody->GetWorldPoint(b2LocalPosition);

        mHeight = currentCarPhysics->mHeight;
        mPhysicsBody->SetTransform(b2WorldPosition, currentCarPhysics->mPhysicsBody->GetAngle());
    }
}

void PedPhysicsBody::HandleFallBegin(float fallDistance)
{
    if (mFalling)
        return;

    mFalling = true;
    mFallDistance = fallDistance;

    b2Vec2 velocity = mPhysicsBody->GetLinearVelocity();
    velocity.Normalize();

    ClearForces();
    mPhysicsBody->SetLinearVelocity(velocity);

    // notify
    PedestrianStateEvent evData { ePedestrianStateEvent_FallFromHeightStart };
    mReferencePed->mStatesManager.ProcessEvent(evData);
}

void PedPhysicsBody::HandleFallEnd()
{
    if (!mFalling)
        return;

    mFalling = false;

    // notify
    PedestrianStateEvent evData { ePedestrianStateEvent_FallFromHeightEnd };
    mReferencePed->mStatesManager.ProcessEvent(evData);
}

void PedPhysicsBody::HandleCarContactBegin()
{
    ++mContactingCars;
}

void PedPhysicsBody::HandleCarContactEnd()
{
    --mContactingCars;

    if (mContactingCars < 0)
    {
        debug_assert(false);
        mContactingCars = 0;
    }
}

void PedPhysicsBody::HandleWaterContact()
{
    if (mWaterContact)
        return;

    mWaterContact = true;

    // notify
    PedestrianStateEvent evData { ePedestrianStateEvent_WaterContact };
    mReferencePed->mStatesManager.ProcessEvent(evData);
}

bool PedPhysicsBody::ShouldContactWith(unsigned int bits) const
{
    if (mReferencePed->IsDead())
    {
        return false;
    }

    debug_assert(bits);
    ePedestrianState currState = mReferencePed->GetCurrentStateID();
    if (currState == ePedestrianState_Falling || currState == ePedestrianState_SlideOnCar ||
        currState == ePedestrianState_EnteringCar || currState == ePedestrianState_ExitingCar)
    {
        return (bits & (PHYSICS_OBJCAT_MAP_SOLID_BLOCK | PHYSICS_OBJCAT_WALL)) > 0;
    }

    if (mReferencePed->mCurrentCar)
    {
        return false;
    }

    if (mReferencePed->IsUnconscious())
    {
        return (bits & PHYSICS_OBJCAT_PED) == 0; // collide to all except for other peds
    }

    return true;    
}

//////////////////////////////////////////////////////////////////////////

CarPhysicsBody::CarPhysicsBody(b2World* physicsWorld, Vehicle* object)
    : PhysicsBody(physicsWorld)
    , mSteeringDirection(CarSteeringDirectionNone)
    , mCarDesc(object->mCarStyle)
    , mAccelerationEnabled(false)
    , mDecelerationEnabled(false)
    , mHandBrakeEnabled()
    , mReferenceCar(object)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.userData = this;
    //bodyDef.linearDamping = 0.15f;
    bodyDef.angularDamping = 2.0f;

    mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(mPhysicsBody);
    
    float shape_size_w = Convert::PixelsToMeters(mCarDesc->mWidth) * 0.5f;
    float shape_size_h = Convert::PixelsToMeters(mCarDesc->mHeight) * 0.5f;

    b2PolygonShape shapeDef;
    shapeDef.SetAsBox(shape_size_h, shape_size_w); // swap h and w

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.0f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_CAR;

    mChassisFixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(mChassisFixture);

    // override mass
    b2MassData massData;
    mPhysicsBody->GetMassData(&massData);
    massData.mass = mCarDesc->mMass;
    mPhysicsBody->SetMassData(&massData);

    SetupWheels();
}

CarPhysicsBody::~CarPhysicsBody()
{
}

void CarPhysicsBody::ResetDriveState()
{
    SetSteering(CarSteeringDirectionNone);
    SetAcceleration(false);
    SetDeceleration(false);
    SetHandBrake(false);
}

void CarPhysicsBody::HandleWaterContact()
{
    if (mWaterContact || mReferenceCar->IsWrecked()) // todo
        return;

    mWaterContact = true;
    // boats aren't receive damage from water
    if (mReferenceCar->mCarStyle->mVType == eCarVType_Boat)
        return;

    mHeight -= 2.0f; // force position underwater
    ClearForces();
    // notify
    mReferenceCar->ReceiveDamageFromWater();
}

void CarPhysicsBody::SimulationStep()
{
    UpdateFriction();
    UpdateDrive();
    UpdateSteer();
}

bool CarPhysicsBody::ShouldContactWith(unsigned int objCatBits) const
{
    return true; // todo
}

void CarPhysicsBody::GetChassisCorners(glm::vec2 corners[4]) const
{
    const b2PolygonShape* shape = (const b2PolygonShape*) mChassisFixture->GetShape();
    debug_assert(shape->m_count == 4);
    for (int icorner = 0; icorner < 4; ++icorner)
    {
        box2d::vec2 point = mPhysicsBody->GetWorldPoint(shape->m_vertices[icorner]);
        corners[icorner] = point;
    }
}

void CarPhysicsBody::GetWheelCorners(eCarWheel wheelID, glm::vec2 corners[4]) const
{
    debug_assert(wheelID < eCarWheel_COUNT);

    const float wheel_size_w = Convert::PixelsToMeters(CAR_WHEEL_SIZE_W_PX) * 0.5f;
    const float wheel_size_h = Convert::PixelsToMeters(CAR_WHEEL_SIZE_H_PX) * 0.5f;
    static const b2Vec2 points[4] =
    {
        b2Vec2(-wheel_size_h, -wheel_size_w),
        b2Vec2( wheel_size_h, -wheel_size_w),
        b2Vec2( wheel_size_h,  wheel_size_w),
        b2Vec2(-wheel_size_h,  wheel_size_w),
    };
    float positionOffset = 0.0f;
    if (wheelID == eCarWheel_Drive)
    {
        positionOffset = mDriveWheelPosition;
    }
    else
    {
        debug_assert(wheelID == eCarWheel_Steer);
        positionOffset = mSteerWheelPosition;
    }

    for (int icorner = 0; icorner < 4; ++icorner)
    {
        b2Vec2 currPoint = points[icorner];
        if (wheelID == eCarWheel_Steer && mSteeringAngleRadians)
        {
           b2Rot rot(mSteeringAngleRadians);
           currPoint = b2Mul(rot, points[icorner]);
        }
        box2d::vec2 point = mPhysicsBody->GetWorldPoint(currPoint + b2Vec2(positionOffset, 0.0f));
        corners[icorner] = point;
    }
}

void CarPhysicsBody::SetSteering(int steerDirection)
{
    mSteeringDirection = steerDirection;
}

void CarPhysicsBody::SetAcceleration(bool isEnabled)
{
    mAccelerationEnabled = isEnabled;
}

void CarPhysicsBody::SetDeceleration(bool isEnabled)
{
    mDecelerationEnabled = isEnabled;
}

void CarPhysicsBody::SetHandBrake(bool isEnabled)
{
    mHandBrakeEnabled = isEnabled;
}

glm::vec2 CarPhysicsBody::GetWheelLateralVelocity(eCarWheel wheelID) const
{
    debug_assert(wheelID < eCarWheel_COUNT);

    box2d::vec2 result_velocity = b2GetWheelLateralVelocity(wheelID);
    return result_velocity;
}

glm::vec2 CarPhysicsBody::GetWheelForwardVelocity(eCarWheel wheelID) const
{
    debug_assert(wheelID < eCarWheel_COUNT);

    box2d::vec2 result_velocity = b2GetWheelForwardVelocity(wheelID);
    return result_velocity;
}

glm::vec2 CarPhysicsBody::GetWheelPosition(eCarWheel wheelID) const
{
    debug_assert(wheelID < eCarWheel_COUNT);

    box2d::vec2 position = b2GetWheelPoint(wheelID);
    return position;
}

glm::vec2 CarPhysicsBody::GetWheelDirection(eCarWheel wheelID) const
{
    debug_assert(wheelID < eCarWheel_COUNT);

    box2d::vec2 direction = b2GetWheelForwardVector(wheelID);
    return direction;
}

float CarPhysicsBody::GetCurrentSpeed() const
{
    b2Vec2 currentForwardNormal = mPhysicsBody->GetWorldVector(box2d::ForwardVector);
    b2Vec2 forwardVelocity = b2Dot(currentForwardNormal, mPhysicsBody->GetLinearVelocity()) * currentForwardNormal;
    return b2Dot(forwardVelocity, currentForwardNormal);
}

void CarPhysicsBody::SetupWheels()
{
    mDriveWheelPosition = Convert::PixelsToMeters(mCarDesc->mDriveWheelOffset);
    mSteerWheelPosition = Convert::PixelsToMeters(mCarDesc->mSteeringWheelOffset);
}

void CarPhysicsBody::UpdateSteer()
{
    //float lockAngle = glm::radians(32.0f);
    //float turnSpeedPerSec = glm::radians(mCarDesc->mTurning * 1.0f);
    //float turnPerTimeStep = (turnSpeedPerSec * gTimeManager.mGameFrameDelta);

    //float desiredAngle = lockAngle * mSteeringDirection;
    //float angleNow = mSteeringAngleRadians;
    //float angleToTurn = b2Clamp((desiredAngle - angleNow), -turnPerTimeStep, turnPerTimeStep);

    //mSteeringAngleRadians = angleNow + angleToTurn;

    // temporary implementation
    float currentSpeed = GetCurrentSpeed();
    float currentVelocity = fabs(currentSpeed);

    if (mSteeringDirection) 
    {
        float torqueForce = 150.0f;
        torqueForce *= (currentSpeed < 0.0f) ? -mSteeringDirection : mSteeringDirection;

        if (currentVelocity < 2.0f) 
        {
            torqueForce *= (currentVelocity / 4.0f);
        }

        mPhysicsBody->ApplyTorque(torqueForce, true);
    }
}

void CarPhysicsBody::UpdateFriction()
{
    b2Vec2 currentForwardNormal = b2GetWheelForwardVelocity(eCarWheel_Drive);
    b2Vec2 impulse = mPhysicsBody->GetMass() * -b2GetWheelLateralVelocity(eCarWheel_Drive);
    mPhysicsBody->ApplyLinearImpulse(impulse, mPhysicsBody->GetWorldCenter(), true);

    if (currentForwardNormal.Length() < 5.99f) 
    {
        mPhysicsBody->ApplyAngularImpulse((0.5f - (currentForwardNormal.Length() / 6.0f) * 0.4f) * mPhysicsBody->GetInertia() * -mPhysicsBody->GetAngularVelocity(), true);
    }
    else 
    {
        mPhysicsBody->ApplyAngularImpulse(0.1f * mPhysicsBody->GetInertia() * -mPhysicsBody->GetAngularVelocity(), true);
    }

    float dragForceMagnitude = -20.0f / (std::min(11.9f, currentForwardNormal.Length()) / 11.9f + 0.1f);
    mPhysicsBody->ApplyForce(dragForceMagnitude * currentForwardNormal, mPhysicsBody->GetWorldCenter(), true);
}

void CarPhysicsBody::UpdateDrive()
{
    float maxForwardSpeed = 100.0f;
    float maxBackwardSpeed = -80.0f;

    float driveForce = 300.0f;
    float brakeForce = 100.0f;
    float reverseForce = 250.0f;

    float desiredSpeed = 0.0f;
    if (mAccelerationEnabled) 
    {
        desiredSpeed = maxForwardSpeed;
    }

    if (mDecelerationEnabled) 
    {
        desiredSpeed = maxBackwardSpeed;
    }

    //find current speed in forward direction
    float currentSpeed = GetCurrentSpeed();

    if (desiredSpeed > 0.0f) 
    {
        mPhysicsBody->ApplyForce(driveForce * b2GetWheelForwardVector(eCarWheel_Drive), mPhysicsBody->GetWorldCenter(), true);
    }
    else if (desiredSpeed < 0.0f) 
    {
        if (GetCurrentSpeed() > 0) 
        {
            mPhysicsBody->ApplyForce(-brakeForce * b2GetWheelForwardVector(eCarWheel_Drive), mPhysicsBody->GetWorldCenter(), true);
        }
        else 
        {
            mPhysicsBody->ApplyForce(-reverseForce * b2GetWheelForwardVector(eCarWheel_Drive), mPhysicsBody->GetWorldCenter(), true);
        }
    }
}

b2Vec2 CarPhysicsBody::b2GetWheelLateralVelocity(eCarWheel wheelID) const
{
    b2Vec2 normal_vector = b2GetWheelLateralVector(wheelID);
    return b2Dot(normal_vector, mPhysicsBody->GetLinearVelocity()) * normal_vector;
}

b2Vec2 CarPhysicsBody::b2GetWheelForwardVelocity(eCarWheel wheelID) const
{
    b2Vec2 normal_vector = b2GetWheelForwardVector(wheelID);
    return b2Dot(normal_vector, mPhysicsBody->GetLinearVelocity()) * normal_vector;
}

b2Vec2 CarPhysicsBody::b2GetWheelForwardVector(eCarWheel wheelID) const
{
    b2Vec2 b2LocalVector = b2GetWheelLocalForwardVector(wheelID);
    return mPhysicsBody->GetWorldVector(b2LocalVector);
}

b2Vec2 CarPhysicsBody::b2GetWheelLateralVector(eCarWheel wheelID) const
{
    b2Vec2 b2LocalVector = b2GetWheelLocalLateralVector(wheelID);
    return mPhysicsBody->GetWorldVector(b2LocalVector);
}

b2Vec2 CarPhysicsBody::b2GetWheelPoint(eCarWheel wheelID) const
{
    b2Vec2 b2LocalPoint = b2GetWheelLocalPoint(wheelID);
    return mPhysicsBody->GetWorldPoint(b2LocalPoint);
}

b2Vec2 CarPhysicsBody::b2GetWheelLocalPoint(eCarWheel wheelID) const
{
    if (wheelID == eCarWheel_Drive)
    {
        return (box2d::ForwardVector * mDriveWheelPosition);
    }
    if (wheelID == eCarWheel_Steer)
    {
        return (box2d::ForwardVector * mSteerWheelPosition);
    }
    debug_assert(false);
    return {};
}

b2Vec2 CarPhysicsBody::b2GetWheelLocalForwardVector(eCarWheel wheelID) const
{
    if (wheelID == eCarWheel_Drive)
    {
        return box2d::ForwardVector;
    }
    if (wheelID == eCarWheel_Steer)
    {
        b2Rot vectorAngle (mSteeringAngleRadians);
        return b2Mul(vectorAngle, box2d::ForwardVector);
    }
    debug_assert(false);
    return {};
}

b2Vec2 CarPhysicsBody::b2GetWheelLocalLateralVector(eCarWheel wheelID) const
{
    if (wheelID == eCarWheel_Drive)
    {
        return box2d::LateralVector;
    }
    if (wheelID == eCarWheel_Steer)
    {
        b2Rot vectorAngle (mSteeringAngleRadians);
        return b2Mul(vectorAngle, box2d::LateralVector);
    }
    debug_assert(false);
    return {};
}

//////////////////////////////////////////////////////////////////////////

ProjectilePhysicsBody::ProjectilePhysicsBody(b2World* physicsWorld, Projectile* object)
    : PhysicsBody(physicsWorld)
    , mReferenceProjectile(object)
{
    debug_assert(object);

    // create body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.fixedRotation = true;
    bodyDef.bullet = true;
    bodyDef.userData = this;

    mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(mPhysicsBody);
    
    debug_assert(object->mWeaponInfo);

    b2CircleShape shapeDef;
    if (object->mWeaponInfo)
    {
        shapeDef.m_radius = object->mWeaponInfo->mProjectileSize;
    }
    else
    {
        debug_assert(false);
        shapeDef.m_radius = 0.1f;
    }

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_PROJECTILE;
    fixtureDef.filter.maskBits = PHYSICS_OBJCAT_PED | PHYSICS_OBJCAT_CAR | PHYSICS_OBJCAT_OBSTACLE | PHYSICS_OBJCAT_MAP_SOLID_BLOCK;

    b2Fixture* b2fixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(b2fixture);
}

ProjectilePhysicsBody::~ProjectilePhysicsBody()
{
}

void ProjectilePhysicsBody::SimulationStep()
{
    if (mReferenceProjectile->IsContactDetected() || mReferenceProjectile->IsMarkedForDeletion())
        return;

    if (mReferenceProjectile->mWeaponInfo == nullptr)
    {
        debug_assert(false);
        return;
    }

    // setup physics
    glm::vec2 velocity = GetSignVector();
    SetLinearVelocity(velocity * mReferenceProjectile->mWeaponInfo->mProjectileSpeed);

    glm::vec2 currPosition = GetPosition2();
    glm::vec2 startPosition(mReferenceProjectile->mStartPosition.x, mReferenceProjectile->mStartPosition.z);

    if (glm::distance(startPosition, currPosition) >= mReferenceProjectile->mWeaponInfo->mBaseHitRange)
    {
        mReferenceProjectile->MarkForDeletion();
    }
}

bool ProjectilePhysicsBody::ShouldContactWith(unsigned int objCatBits) const
{
    if (mReferenceProjectile->IsContactDetected() || mReferenceProjectile->IsMarkedForDeletion())
        return false;

    return true;
}
