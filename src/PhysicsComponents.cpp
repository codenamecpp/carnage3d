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

void PhysicsComponent::AddLinearImpulse(const glm::vec2& impulse)
{
    box2d::vec2 b2Impulse = impulse;
    mPhysicsBody->ApplyLinearImpulseToCenter(b2Impulse, true);
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
    , mCurrentTraction(1.0f)
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
    //fixtureDef.friction = 1.0f;
    //fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_CAR;

    mChassisFixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(mChassisFixture);
    SetPosition(startPosition, startRotation);

    SetupWheels();
}

CarPhysicsComponent::~CarPhysicsComponent()
{
    FreeWheels();
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
    UpdateWheelFriction(eCarWheelID_Steering);

    UpdateWheelDrive(eCarWheelID_Drive);
    UpdateWheelDrive(eCarWheelID_Steering);

    UpdateSteering();
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

    const WheelData& wheel = mCarWheels[wheelID];
    if (wheel.mBody == nullptr)
    {
        debug_assert(false);
        return;
    }

    const b2PolygonShape* shape = (const b2PolygonShape*) wheel.mFixture->GetShape();
    debug_assert(shape->m_count == 4);
    for (int icorner = 0; icorner < 4; ++icorner)
    {
        box2d::vec2 point = wheel.mBody->GetWorldPoint(shape->m_vertices[icorner]);
        corners[icorner] = point;
    }
}

bool CarPhysicsComponent::HasWheel(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    return mCarWheels[wheelID].mBody != nullptr;
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

    box2d::vec2 result_velocity;

    const WheelData& wheel = mCarWheels[wheelID];
    if (wheel.mBody)
    {
        result_velocity = GetWheelLateralVelocity(wheel.mBody);
    }
    else
    {
        debug_assert(false);
    }
    return result_velocity;
}

glm::vec2 CarPhysicsComponent::GetWheelForwardVelocity(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    box2d::vec2 result_velocity;

    const WheelData& wheel = mCarWheels[wheelID];
    if (wheel.mBody)
    {
        result_velocity = GetWheelForwardVelocity(wheel.mBody);
    }
    else
    {
        debug_assert(false);
    }
    return result_velocity;
}

glm::vec2 CarPhysicsComponent::GetWheelPosition(eCarWheelID wheelID) const
{
    debug_assert(wheelID < eCarWheelID_COUNT);

    box2d::vec2 position;

    const WheelData& wheel = mCarWheels[wheelID];
    if (wheel.mBody)
    {
        position = wheel.mBody->GetWorldCenter();
    }
    else
    {
        debug_assert(false);
    }
    return position;
}

void CarPhysicsComponent::SetupWheels()
{
    CreateWheel(eCarWheelID_Steering);
    CreateWheel(eCarWheelID_Drive);

    // setup joints
    b2RevoluteJointDef jointDef;
    jointDef.enableLimit = true;
    jointDef.lowerAngle = 0.0f;
    jointDef.upperAngle = 0.0f;

    // front wheel
    {
        WheelData& wheel = mCarWheels[eCarWheelID_Steering];
	    jointDef.Initialize(mPhysicsBody, wheel.mBody, wheel.mBody->GetPosition());
        jointDef.localAnchorB.SetZero();
	    mFrontWheelJoint = (b2RevoluteJoint*)mPhysicsWorld->CreateJoint(&jointDef);
    }

    // rear wheel
    {
        WheelData& wheel = mCarWheels[eCarWheelID_Drive];
	    jointDef.Initialize(mPhysicsBody, wheel.mBody, wheel.mBody->GetPosition());
        jointDef.localAnchorB.SetZero();
	    mRearWheelJoint = (b2RevoluteJoint*)mPhysicsWorld->CreateJoint(&jointDef);
    }

    mPhysicsBody->ResetMassData();
}

void CarPhysicsComponent::FreeWheels()
{
    // destroy joints
    if (mFrontWheelJoint)
    {
        mPhysicsWorld->DestroyJoint(mFrontWheelJoint);
        mFrontWheelJoint = nullptr;
    }

    if (mRearWheelJoint)
    {
        mPhysicsWorld->DestroyJoint(mRearWheelJoint);
        mRearWheelJoint = nullptr;
    }

    for (WheelData& currWheel: mCarWheels)
    {
        if (currWheel.mBody == nullptr)
            continue;

        mPhysicsWorld->DestroyBody(currWheel.mBody);

        currWheel.mBody = nullptr;
        currWheel.mFixture = nullptr;
    }
}

void CarPhysicsComponent::CreateWheel(eCarWheelID wheelID)
{
    WheelData& wheel = mCarWheels[wheelID];

    debug_assert(wheel.mBody == nullptr && wheel.mFixture == nullptr);

    // todo: move it somewhere else
    const int wheel_pixels_w = 6;
    const int wheel_pixels_h = 12;

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.userData = this;

    // fix position
    if (wheelID == eCarWheelID_Steering)
    {
        bodyDef.position.x = Convert::PixelsToMeters(mCarDesc->mSteeringWheelOffset);
    }
    else if (wheelID == eCarWheelID_Drive)
    {
        bodyDef.position.x = Convert::PixelsToMeters(mCarDesc->mDriveWheelOffset);
    }
    else
    {
        debug_assert(false);
    }

    bodyDef.position = mPhysicsBody->GetWorldPoint(bodyDef.position);

    wheel.mBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(wheel.mBody);
    wheel.mBody->SetTransform(bodyDef.position, mPhysicsBody->GetAngle());

    float wheel_size_w = Convert::PixelsToMeters(wheel_pixels_w) * 0.5f;
    float wheel_size_h = Convert::PixelsToMeters(wheel_pixels_h) * 0.5f;
    
    b2PolygonShape shapeDef;
    shapeDef.SetAsBox(wheel_size_h, wheel_size_w); // swap h and w

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 1.0f;
    fixtureDef.filter.categoryBits = 0; // no collisions

    wheel.mFixture = wheel.mBody->CreateFixture(&fixtureDef);
    debug_assert(wheel.mFixture);
}

void CarPhysicsComponent::UpdateSteering()
{
    float lockAngle = glm::radians(32.0f);
    float turnSpeedPerSec = glm::radians(mCarDesc->mTurning * 1.0f);
    float turnPerTimeStep = (turnSpeedPerSec * gTimeManager.mGameFrameDelta);

    float desiredAngle = lockAngle * mSteeringDirection;

    float angleNow = mFrontWheelJoint->GetJointAngle();
    float angleToTurn = desiredAngle - angleNow;

    angleToTurn = b2Clamp(angleToTurn, -turnPerTimeStep, turnPerTimeStep);

    float newAngle = angleNow + angleToTurn;
    mFrontWheelJoint->SetLimits(newAngle, newAngle);
}

void CarPhysicsComponent::UpdateWheelFriction(eCarWheelID wheelID)
{
    const WheelData& wheel = mCarWheels[wheelID];
    float maxLateralImpulse = 8.0f; // todo: magic numbers

    //lateral linear velocity
    b2Vec2 impulse = mPhysicsBody->GetMass() * -GetWheelLateralVelocity(wheel.mBody);
    if (impulse.Length() > maxLateralImpulse)
    {
        impulse *= maxLateralImpulse / impulse.Length();
    }
    wheel.mBody->ApplyLinearImpulse(mCurrentTraction * impulse, wheel.mBody->GetWorldCenter(), true);

    //angular velocity
    wheel.mBody->ApplyAngularImpulse(mCurrentTraction * 0.1f * wheel.mBody->GetInertia() * -wheel.mBody->GetAngularVelocity(), true);

    //forward linear velocity
    b2Vec2 currentForwardNormal = GetWheelForwardVelocity(wheel.mBody);
    float currentForwardSpeed = currentForwardNormal.Normalize();
    float dragForceMagnitude = -2.0f * currentForwardSpeed;
    wheel.mBody->ApplyForce(mCurrentTraction * dragForceMagnitude * currentForwardNormal, wheel.mBody->GetWorldCenter(), true);
}

void CarPhysicsComponent::UpdateWheelDrive(eCarWheelID wheelID)
{
    const WheelData& wheel = mCarWheels[wheelID];

    float desiredSpeed = 0.0f;
    if (mAccelerationEnabled)
    {
        desiredSpeed += (mCarDesc->mMaxSpeed) * 1.0f;
    }
    if (mDecelerationEnabled)
    {
        desiredSpeed += (mCarDesc->mMinSpeed) * 1.0f;
    }
    if (mHandBrakeEnabled)
    {
        desiredSpeed = 0.0f;
    }

    float maxDriveForce = mCarDesc->mAcceleration;

    // find current speed in forward direction
    b2Vec2 currentForwardNormal = wheel.mBody->GetWorldVector(box2d::ForwardVector);

    float currentSpeed = b2Dot(GetWheelForwardVelocity(wheel.mBody), currentForwardNormal);
    float force = 0;

    if (fabs(desiredSpeed - currentSpeed) < 0.001f)
        return;

    if (desiredSpeed > currentSpeed)
    {
        force = maxDriveForce;
    }
    else
    {
        force = -maxDriveForce;
    }

    b2Vec2 forceVec = mCurrentTraction * force * currentForwardNormal;
    wheel.mBody->ApplyForce(forceVec, wheel.mBody->GetWorldCenter(), true);
}

b2Vec2 CarPhysicsComponent::GetWheelLateralVelocity(b2Body* carWheel) const
{
	b2Vec2 right_normal = carWheel->GetWorldVector(box2d::LateralVector);
    return b2Dot(right_normal, carWheel->GetLinearVelocity()) * right_normal;
}

b2Vec2 CarPhysicsComponent::GetWheelForwardVelocity(b2Body* carWheel) const
{
	b2Vec2 forward_normal = carWheel->GetWorldVector(box2d::ForwardVector);
    return b2Dot(forward_normal, carWheel->GetLinearVelocity()) * forward_normal;
}