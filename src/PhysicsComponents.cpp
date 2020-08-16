#include "stdafx.h"
#include "PhysicsComponents.h"
#include "PhysicsDefs.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "PhysicsManager.h"
#include "TimeManager.h"
#include "Box2D_Helpers.h"

PhysicsComponent::PhysicsComponent(b2World* physicsWorld)
    : mHeight()
    , mPhysicsWorld(physicsWorld)
    , mPhysicsBody()
    , mPreviousPosition()
    , mSmoothPosition()
{
    debug_assert(physicsWorld);
}

PhysicsComponent::~PhysicsComponent()
{
}

void PhysicsComponent::SetPosition(const glm::vec3& position)
{
    mHeight = position.y;
    mPreviousPosition = position;
    mSmoothPosition = position;

    box2d::vec2 b2position { position.x, position.z };
    mPhysicsBody->SetTransform(b2position, mPhysicsBody->GetAngle());
}

void PhysicsComponent::SetPosition(const glm::vec3& position, cxx::angle_t rotationAngle)
{
    mHeight = position.y;
    mPreviousPosition = position;
    mSmoothPosition = position;

    box2d::vec2 b2position { position.x, position.z };
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
    box2d::vec2 b2Force = force;
    mPhysicsBody->ApplyForceToCenter(b2Force, true);
}

void PhysicsComponent::AddForce(const glm::vec2& force, const glm::vec2& position)
{
    box2d::vec2 b2Force = force;
    box2d::vec2 b2Position = position;
    mPhysicsBody->ApplyForce(b2Force, b2Position, true);
}

void PhysicsComponent::AddLinearImpulse(const glm::vec2& impulse)
{
    box2d::vec2 b2Impulse = impulse;
    mPhysicsBody->ApplyLinearImpulseToCenter(b2Impulse, true);
}

void PhysicsComponent::AddLinearImpulse(const glm::vec2& impulse, const glm::vec2& position)
{
    box2d::vec2 b2Impulse = impulse;
    box2d::vec2 b2Position = position;
    mPhysicsBody->ApplyLinearImpulse(b2Impulse, b2Position, true);
}

glm::vec3 PhysicsComponent::GetPosition() const
{
    const b2Vec2& b2position = mPhysicsBody->GetPosition();
    return { b2position.x, mHeight, b2position.y };
}

glm::vec2 PhysicsComponent::GetLinearVelocity() const
{
    const b2Vec2& b2position = mPhysicsBody->GetLinearVelocity();
    return { b2position.x, b2position.y };
}

cxx::angle_t PhysicsComponent::GetAngularVelocity() const
{
    cxx::angle_t angularVelocity = cxx::angle_t::from_radians(mPhysicsBody->GetAngularVelocity());
    return angularVelocity;
}

void PhysicsComponent::AddAngularImpulse(float impulse)
{
    mPhysicsBody->ApplyAngularImpulse(impulse, true);
}

void PhysicsComponent::SetAngularVelocity(cxx::angle_t angularVelocity)
{
    mPhysicsBody->SetAngularVelocity(angularVelocity.to_radians());
}

void PhysicsComponent::SetLinearVelocity(const glm::vec2& velocity)
{
    box2d::vec2 b2vec = velocity;
    mPhysicsBody->SetLinearVelocity(b2vec);
}

void PhysicsComponent::ClearForces()
{
    mPhysicsBody->SetLinearVelocity(box2d::NullVector);
    mPhysicsBody->SetAngularVelocity(0.0f);
}

glm::vec2 PhysicsComponent::GetSignVector() const
{
    float angleRadians = mPhysicsBody->GetAngle();
    return { 
        cos(angleRadians), 
        sin(angleRadians)
    };
}

glm::vec2 PhysicsComponent::GetWorldPoint(const glm::vec2& localPosition) const
{
    box2d::vec2 b2LocalPosition = localPosition;
    box2d::vec2 b2WorldPosition = mPhysicsBody->GetWorldPoint(b2LocalPosition);

    return b2WorldPosition;
}

glm::vec2 PhysicsComponent::GetLocalPoint(const glm::vec2& worldPosition) const
{
    box2d::vec2 b2WorldPosition = worldPosition;
    box2d::vec2 b2LocalPosition = mPhysicsBody->GetLocalPoint(b2WorldPosition);

    return b2LocalPosition;
}

glm::vec2 PhysicsComponent::GetWorldVector(const glm::vec2& localVector) const
{
    box2d::vec2 b2LocalVector = localVector;
    box2d::vec2 b2WorldVector = mPhysicsBody->GetWorldVector(b2LocalVector);

    return b2WorldVector;
}

glm::vec2 PhysicsComponent::GetLocalVector(const glm::vec2& worldVector) const
{
    box2d::vec2 b2WorldVector = worldVector;
    box2d::vec2 b2LocalVector = mPhysicsBody->GetLocalVector(b2WorldVector);

    return b2LocalVector;
}

void PhysicsComponent::SetRespawned()
{
    mFalling = false;
    mWaterContact = false;
    mFallDistance = 0.0f;
}

//////////////////////////////////////////////////////////////////////////

PedPhysicsComponent::PedPhysicsComponent(b2World* physicsWorld, const glm::vec3& startPosition, cxx::angle_t startRotation)
    : PhysicsComponent(physicsWorld)
    , mPhysicsComponentsListNode(this)
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

    SetPosition(startPosition, startRotation);
}

PedPhysicsComponent::~PedPhysicsComponent()
{
}

void PedPhysicsComponent::SimulationStep()
{
    if (mReferencePed->mCurrentCar)
    {
        CarPhysicsComponent* currentCarPhysics = mReferencePed->mCurrentCar->mPhysicsComponent;

        box2d::vec2 b2LocalPosition = mCarPointLocal;
        box2d::vec2 b2WorldPosition = currentCarPhysics->mPhysicsBody->GetWorldPoint(b2LocalPosition);

        mHeight = currentCarPhysics->mHeight;
        mPhysicsBody->SetTransform(b2WorldPosition, currentCarPhysics->mPhysicsBody->GetAngle());
    }
}

void PedPhysicsComponent::HandleFallBegin(float fallDistance)
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

void PedPhysicsComponent::HandleFallEnd()
{
    if (!mFalling)
        return;

    mFalling = false;

    // notify
    PedestrianStateEvent evData { ePedestrianStateEvent_FallFromHeightEnd };
    mReferencePed->mStatesManager.ProcessEvent(evData);
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

void PedPhysicsComponent::HandleWaterContact()
{
    if (mWaterContact)
        return;

    mWaterContact = true;

    // notify
    PedestrianStateEvent evData { ePedestrianStateEvent_WaterContact };
    mReferencePed->mStatesManager.ProcessEvent(evData);
}

bool PedPhysicsComponent::ShouldCollideWith(unsigned int bits) const
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

CarPhysicsComponent::CarPhysicsComponent(b2World* physicsWorld, CarStyle* desc, const glm::vec3& startPosition, cxx::angle_t startRotation)
    : PhysicsComponent(physicsWorld)
    , mPhysicsComponentsListNode(this)
    , mSteeringDirection(CarSteeringDirectionNone)
    , mCarDesc(desc)
    , mAccelerationEnabled(false)
    , mDecelerationEnabled(false)
    , mHandBrakeEnabled()
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.userData = this;
    //bodyDef.linearDamping = 0.15f;
    bodyDef.angularDamping = 2.0f;

    mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(mPhysicsBody);
    //physicsObject->mDepth = (1.0f * desc->mDepth) / MAP_PIXELS_PER_TILE;
    
    float shape_size_w = Convert::PixelsToMeters(desc->mWidth) * 0.5f;
    float shape_size_h = Convert::PixelsToMeters(desc->mHeight) * 0.5f;

    b2PolygonShape shapeDef;
    shapeDef.SetAsBox(shape_size_h, shape_size_w); // swap h and w

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.0f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_CAR;

    mChassisFixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(mChassisFixture);
    SetPosition(startPosition, startRotation);

    // override mass
    b2MassData massData;
    mPhysicsBody->GetMassData(&massData);
    massData.mass = mCarDesc->mMass;
    mPhysicsBody->SetMassData(&massData);

    SetupWheels();
}

CarPhysicsComponent::~CarPhysicsComponent()
{
}

void CarPhysicsComponent::ResetDriveState()
{
    SetSteering(CarSteeringDirectionNone);
    SetAcceleration(false);
    SetDeceleration(false);
    SetHandBrake(false);
}

void CarPhysicsComponent::HandleWaterContact()
{
    if (mWaterContact || mReferenceCar->mDead) // todo
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

void CarPhysicsComponent::SimulationStep()
{
    UpdateWheelFriction(eCarWheelID_Drive);
    UpdateWheelFriction(eCarWheelID_Steer);

    UpdateDrive();
    UpdateSteer();
}

void CarPhysicsComponent::GetChassisCorners(glm::vec2 corners[4]) const
{
    const b2PolygonShape* shape = (const b2PolygonShape*) mChassisFixture->GetShape();
    debug_assert(shape->m_count == 4);
    for (int icorner = 0; icorner < 4; ++icorner)
    {
        box2d::vec2 point = mPhysicsBody->GetWorldPoint(shape->m_vertices[icorner]);
        corners[icorner] = point;
    }
}

void CarPhysicsComponent::GetWheelCorners(eCarWheelID wheelID, glm::vec2 corners[4]) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

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
    if (wheelID == eCarWheelID_Drive)
    {
        positionOffset = mDriveWheelPosition;
    }
    else
    {
        debug_assert(wheelID == eCarWheelID_Steer);
        positionOffset = mSteerWheelPosition;
    }

    for (int icorner = 0; icorner < 4; ++icorner)
    {
        b2Vec2 currPoint = points[icorner];
        if (wheelID == eCarWheelID_Steer && mSteeringAngleRadians)
        {
           b2Rot rot(mSteeringAngleRadians);
           currPoint = b2Mul(rot, points[icorner]);
        }
        box2d::vec2 point = mPhysicsBody->GetWorldPoint(currPoint + b2Vec2(positionOffset, 0.0f));
        corners[icorner] = point;
    }
}

void CarPhysicsComponent::SetSteering(int steerDirection)
{
    mSteeringDirection = steerDirection;
}

void CarPhysicsComponent::SetAcceleration(bool isEnabled)
{
    mAccelerationEnabled = isEnabled;
}

void CarPhysicsComponent::SetDeceleration(bool isEnabled)
{
    mDecelerationEnabled = isEnabled;
}

void CarPhysicsComponent::SetHandBrake(bool isEnabled)
{
    mHandBrakeEnabled = isEnabled;
}

glm::vec2 CarPhysicsComponent::GetWheelLateralVelocity(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    box2d::vec2 result_velocity = b2GetWheelLateralVelocity(wheelID);
    return result_velocity;
}

glm::vec2 CarPhysicsComponent::GetWheelForwardVelocity(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    box2d::vec2 result_velocity = b2GetWheelForwardVelocity(wheelID);
    return result_velocity;
}

glm::vec2 CarPhysicsComponent::GetWheelPosition(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    b2Vec2 b2Position = b2GetWheelLocalPosition(wheelID);

    box2d::vec2 position = mPhysicsBody->GetWorldPoint(b2Position);
    return position;
}

glm::vec2 CarPhysicsComponent::GetWheelDirection(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    b2Vec2 b2Direction = b2GetWheelLocalForwardVector(wheelID);

    box2d::vec2 direction = mPhysicsBody->GetWorldVector(b2Direction);
    return direction;
}

float CarPhysicsComponent::GetCurrentVelocity() const
{
    b2Vec2 currentForwardNormal = mPhysicsBody->GetWorldVector(box2d::ForwardVector);
    b2Vec2 forwardVelocity = b2Dot(currentForwardNormal, mPhysicsBody->GetLinearVelocity()) * currentForwardNormal;
    return b2Dot(forwardVelocity, currentForwardNormal);
}

void CarPhysicsComponent::SetupWheels()
{
    mDriveWheelPosition = Convert::PixelsToMeters(mCarDesc->mDriveWheelOffset);
    mSteerWheelPosition = Convert::PixelsToMeters(mCarDesc->mSteeringWheelOffset);
}

void CarPhysicsComponent::UpdateSteer()
{
    float lockAngle = glm::radians(32.0f);
    float turnSpeedPerSec = glm::radians(mCarDesc->mTurning * 1.0f);
    float turnPerTimeStep = (turnSpeedPerSec * gTimeManager.mGameFrameDelta);

    float desiredAngle = lockAngle * mSteeringDirection;
    float angleNow = mSteeringAngleRadians;
    float angleToTurn = desiredAngle - angleNow;

    angleToTurn = b2Clamp(angleToTurn, -turnPerTimeStep, turnPerTimeStep);

    mSteeringAngleRadians = angleNow + angleToTurn;
}

void CarPhysicsComponent::UpdateWheelFriction(eCarWheelID wheelID)
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    
}

void CarPhysicsComponent::UpdateDrive()
{
    if (mSteeringAngleRadians)
    {
        mPhysicsBody->ApplyTorque(mSteeringAngleRadians * mCarDesc->mTurnRatio, true);
    }

    float desiredSpeed = 0.0f;
    if (mAccelerationEnabled)
    {
        desiredSpeed = Convert::MapUnitsToMeters(mCarDesc->mThrust * 1.0f);
    }
    if (mDecelerationEnabled)
    {
        desiredSpeed = -Convert::MapUnitsToMeters(mCarDesc->mThrust * 1.5f);
    }
    
    if (desiredSpeed)
    {
        b2Vec2 b2Force = mPhysicsBody->GetWorldVector(b2GetWheelLocalForwardVector(eCarWheelID_Steer));
        b2Force *= desiredSpeed;

        b2Vec2 b2Pos = mPhysicsBody->GetWorldPoint(b2GetWheelLocalPosition(eCarWheelID_Steer));
        mPhysicsBody->ApplyForce(b2Force, b2Pos, true);
    }

    KillOrthogonalVelocity(0.1f);
}

void CarPhysicsComponent::KillOrthogonalVelocity(float drift)
{
    b2Vec2 currentVelocity = mPhysicsBody->GetLinearVelocity();
    b2Vec2 forward = b2GetWheelLocalForwardVector(eCarWheelID_Drive);
    forward = mPhysicsBody->GetWorldVector(forward);

    b2Vec2 right = b2GetWheelLocalLateralVector(eCarWheelID_Drive);
    right = mPhysicsBody->GetWorldVector(right);

    b2Vec2 forwardVelocity = forward;
    forwardVelocity *= b2Dot(currentVelocity, forward);
    b2Vec2 rightVelocity = right;
    rightVelocity *= b2Dot(currentVelocity, right);
    rightVelocity *= drift;

    mPhysicsBody->SetLinearVelocity(forwardVelocity + rightVelocity);
}

b2Vec2 CarPhysicsComponent::b2GetWheelLateralVelocity(eCarWheelID wheelID) const
{
    b2Vec2 normal_vector = b2GetWheelLocalLateralVector(wheelID);

    normal_vector = mPhysicsBody->GetWorldVector(normal_vector);
    return b2Dot(normal_vector, mPhysicsBody->GetLinearVelocity()) * normal_vector;
}

b2Vec2 CarPhysicsComponent::b2GetWheelForwardVelocity(eCarWheelID wheelID) const
{
    b2Vec2 normal_vector = b2GetWheelLocalForwardVector(wheelID);

    normal_vector = mPhysicsBody->GetWorldVector(normal_vector);
    return b2Dot(normal_vector, mPhysicsBody->GetLinearVelocity()) * normal_vector;
}

b2Vec2 CarPhysicsComponent::b2GetWheelLocalPosition(eCarWheelID wheelID) const
{
    if (wheelID == eCarWheelID_Drive)
    {
        return (box2d::ForwardVector * mDriveWheelPosition);
    }

    if (wheelID == eCarWheelID_Steer)
    {
        return (box2d::ForwardVector * mSteerWheelPosition);
    }
    debug_assert(false);
    return {};
}

b2Vec2 CarPhysicsComponent::b2GetWheelLocalForwardVector(eCarWheelID wheelID) const
{
    if (wheelID == eCarWheelID_Drive)
    {
        return box2d::ForwardVector;
    }

    if (wheelID == eCarWheelID_Steer)
    {
        b2Rot vectorAngle (mSteeringAngleRadians);
        return b2Mul(vectorAngle, box2d::ForwardVector);
    }
    debug_assert(false);
    return {};
}

b2Vec2 CarPhysicsComponent::b2GetWheelLocalLateralVector(eCarWheelID wheelID) const
{
    if (wheelID == eCarWheelID_Drive)
    {
        return box2d::LateralVector;
    }

    if (wheelID == eCarWheelID_Steer)
    {
        b2Rot vectorAngle (mSteeringAngleRadians);
        return b2Mul(vectorAngle, box2d::LateralVector);
    }
    debug_assert(false);
    return {};
}