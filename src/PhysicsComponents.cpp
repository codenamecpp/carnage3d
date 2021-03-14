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
#include "CarnageGame.h"

PhysicsBody::PhysicsBody(b2World* physicsWorld)
    : mHeight()
    , mPhysicsWorld(physicsWorld)
    , mPhysicsBody()
    , mPreviousPosition()
    , mSmoothPosition()
    , mPreviousRotation()
    , mSmoothRotation()
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
    mPreviousRotation = rotationAngle;
    mSmoothRotation = rotationAngle;

    box2d::vec2 b2position { position.x, position.z };
    mPhysicsBody->SetTransform(b2position, rotationAngle.to_radians());
}

void PhysicsBody::SetRotationAngle(cxx::angle_t rotationAngle)
{
    mPreviousRotation = rotationAngle;
    mSmoothRotation = rotationAngle;

    mPhysicsBody->SetTransform(mPhysicsBody->GetPosition(), rotationAngle.to_radians());
}

cxx::angle_t PhysicsBody::GetRotationAngle() const
{
    cxx::angle_t rotationAngle = cxx::angle_t::from_radians(mPhysicsBody->GetAngle());
    return rotationAngle;
}

void PhysicsBody::SetOrientation2(const glm::vec2& signDirection)
{
    float rotationAngleRadians = atan2f(signDirection.y, signDirection.x);
    mPhysicsBody->SetTransform(mPhysicsBody->GetPosition(), rotationAngleRadians);
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

glm::vec2 PhysicsBody::GetLinearVelocityFromWorldPoint(const glm::vec2& worldPosition) const
{
    box2d::vec2 b2point = worldPosition;
    box2d::vec2 b2vec = mPhysicsBody->GetLinearVelocityFromWorldPoint(b2point);
    return b2vec;
}

glm::vec2 PhysicsBody::GetLinearVelocityFromLocalPoint(const glm::vec2& localPosition) const
{
    box2d::vec2 b2point = localPosition;
    box2d::vec2 b2vec = mPhysicsBody->GetLinearVelocityFromLocalPoint(b2point);
    return b2vec;
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

//////////////////////////////////////////////////////////////////////////

PedestrianPhysics::PedestrianPhysics(b2World* physicsWorld, Pedestrian* object)
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

void PedestrianPhysics::SimulationStep()
{
    if (mReferencePed->mCurrentCar)
    {
        CarPhysics* currentCarPhysics = mReferencePed->mCurrentCar->mPhysicsBody;

        box2d::vec2 b2LocalPosition = mCarPointLocal;
        box2d::vec2 b2WorldPosition = currentCarPhysics->mPhysicsBody->GetWorldPoint(b2LocalPosition);

        mHeight = currentCarPhysics->mHeight;
        mPhysicsBody->SetTransform(b2WorldPosition, currentCarPhysics->mPhysicsBody->GetAngle());
    }
}

void PedestrianPhysics::HandleFallBegin()
{
    if (mFalling)
        return;

    mFalling = true;
    mFallStartHeight = mHeight;

    b2Vec2 velocity = mPhysicsBody->GetLinearVelocity();
    velocity.Normalize();

    ClearForces();
    mPhysicsBody->SetLinearVelocity(velocity);

    // notify
    PedestrianStateEvent evData { ePedestrianStateEvent_FallFromHeightStart };
    mReferencePed->mStatesManager.ProcessEvent(evData);
}

void PedestrianPhysics::HandleFallEnd()
{
    if (!mFalling)
        return;

    mFalling = false;

    // notify
    PedestrianStateEvent evData { ePedestrianStateEvent_FallFromHeightEnd };
    mReferencePed->mStatesManager.ProcessEvent(evData);

    // damage
    if (mFallStartHeight > mHeight)
    {
        DamageInfo damageInfo;
        damageInfo.SetDamageFromFall(mFallStartHeight - mHeight);
        mReferencePed->ReceiveDamage(damageInfo);
    }
}

void PedestrianPhysics::HandleCarContactBegin()
{
    ++mContactingCars;
}

void PedestrianPhysics::HandleCarContactEnd()
{
    --mContactingCars;

    if (mContactingCars < 0)
    {
        debug_assert(false);
        mContactingCars = 0;
    }
}

void PedestrianPhysics::HandleWaterContact()
{
    if (mWaterContact)
        return;    

    // notify
    PedestrianStateEvent evData { ePedestrianStateEvent_WaterContact };
    if (mReferencePed->mStatesManager.ProcessEvent(evData))
    {
        mWaterContact = true;
        mFalling = false;

        // create effect
        Decoration* splashEffect = gGameObjectsManager.CreateWaterSplash(GetPosition());
        debug_assert(splashEffect);

        mHeight -= Convert::MapUnitsToMeters(1.0f); // put it down
    }
}

bool PedestrianPhysics::ShouldContactWith(unsigned int bits) const
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

    if (mReferencePed->IsStunned())
    {
        return (bits & PHYSICS_OBJCAT_PED) == 0; // collide to all except for other peds
    }

    return true;    
}

//////////////////////////////////////////////////////////////////////////

CarPhysics::CarPhysics(b2World* physicsWorld, Vehicle* object)
    : PhysicsBody(physicsWorld)
    , mCarDesc(object->mCarInfo)
    , mReferenceCar(object)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.userData = this;

    mPhysicsBody = mPhysicsWorld->CreateBody(&bodyDef);
    debug_assert(mPhysicsBody);
    
    b2PolygonShape shapeDef;
    shapeDef.SetAsBox(mCarDesc->mDimensions.z * 0.5f, mCarDesc->mDimensions.x * 0.5f); // swap z and x

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 80.0f;
    fixtureDef.friction = 0.0f;
    fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = PHYSICS_OBJCAT_CAR;

    mChassisFixture = mPhysicsBody->CreateFixture(&fixtureDef);
    debug_assert(mChassisFixture);

    mRearTireOffset = Convert::PixelsToMeters(mCarDesc->mDriveWheelOffset);
    mFrontTireOffset = Convert::PixelsToMeters(mCarDesc->mSteeringWheelOffset);

    float mass = mPhysicsBody->GetMass();
    int bp = 0;
}

void CarPhysics::HandleWaterContact()
{
    if (mWaterContact)
        return;

    mWaterContact = true;
    mFalling = false;

    // boats aren't receive damage from water
    if (mReferenceCar->mCarInfo->mClassID == eVehicleClass_Boat)
        return;

    ClearForces();
    if (!mReferenceCar->IsWrecked())
    {
        DamageInfo damageInfo;
        damageInfo.mDamageCause = eDamageCause_Drowning;
        mReferenceCar->ReceiveDamage(damageInfo);
    }

    // create effect
    glm::vec2 splashPoints[5];
    GetChassisCorners(splashPoints);
    splashPoints[4] = GetPosition2();
    for (const glm::vec2& currPoint: splashPoints)
    {
        Decoration* splashEffect = gGameObjectsManager.CreateWaterSplash(glm::vec3(currPoint.x, mHeight, currPoint.y));
        debug_assert(splashEffect);
    }
    mHeight -= Convert::MapUnitsToMeters(1.0f); // put it down
}

void CarPhysics::SimulationStep()
{
    DriveCtlState currCtlState;

    if (!mReferenceCar->IsWrecked())
    {
        Pedestrian* carDriver = mReferenceCar->GetCarDriver();
        if (carDriver)
        {
            currCtlState.mDriveDirection = carDriver->mCtlState.mAcceleration;
            currCtlState.mSteerDirection = carDriver->mCtlState.mSteerDirection;
            currCtlState.mHandBrake = carDriver->mCtlState.mHandBrake;
        }
    }

    UpdateFriction(currCtlState);
    UpdateDrive(currCtlState);
    UpdateSteer(currCtlState);
}

bool CarPhysics::ShouldContactWith(unsigned int objCatBits) const
{
    return true; // todo
}

void CarPhysics::GetChassisCorners(glm::vec2 corners[4]) const
{
    const b2PolygonShape* shape = (const b2PolygonShape*) mChassisFixture->GetShape();
    debug_assert(shape->m_count == 4);
    for (int icorner = 0; icorner < 4; ++icorner)
    {
        box2d::vec2 point = mPhysicsBody->GetWorldPoint(shape->m_vertices[icorner]);
        corners[icorner] = point;
    }
}

void CarPhysics::GetLocalChassisCorners(glm::vec2 corners[4]) const
{
    const b2PolygonShape* shape = (const b2PolygonShape*) mChassisFixture->GetShape();
    debug_assert(shape->m_count == 4);
    for (int icorner = 0; icorner < 4; ++icorner)
    {
        box2d::vec2 point = shape->m_vertices[icorner];
        corners[icorner] = point;
    }
}

void CarPhysics::GetTireCorners(eCarTire tireID, glm::vec2 corners[4]) const
{
    debug_assert(tireID < eCarTire_COUNT);

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
    if (tireID == eCarTire_Rear)
    {
        positionOffset = mRearTireOffset;
    }
    else
    {
        debug_assert(tireID == eCarTire_Front);
        positionOffset = mFrontTireOffset;
    }

    for (int icorner = 0; icorner < 4; ++icorner)
    {
        b2Vec2 currPoint = points[icorner];
        if (tireID == eCarTire_Front && mSteeringAngleRadians)
        {
           b2Rot rot(mSteeringAngleRadians);
           currPoint = b2Mul(rot, points[icorner]);
        }
        box2d::vec2 point = mPhysicsBody->GetWorldPoint(currPoint + b2Vec2(positionOffset, 0.0f));
        corners[icorner] = point;
    }
}

glm::vec2 CarPhysics::GetTireLateralVelocity(eCarTire tireID) const
{
    debug_assert(tireID < eCarTire_COUNT);

    box2d::vec2 result_velocity = b2GetTireLateralVelocity(tireID);
    return result_velocity;
}

glm::vec2 CarPhysics::GetTireForwardVelocity(eCarTire tireID) const
{
    debug_assert(tireID < eCarTire_COUNT);

    box2d::vec2 result_velocity = b2GetTireForwardVelocity(tireID);
    return result_velocity;
}

glm::vec2 CarPhysics::GetTirePosition(eCarTire tireID) const
{
    debug_assert(tireID < eCarTire_COUNT);

    box2d::vec2 position = b2GetTirePos(tireID);
    return position;
}

glm::vec2 CarPhysics::GetTireForward(eCarTire tireID) const
{
    debug_assert(tireID < eCarTire_COUNT);

    box2d::vec2 direction = b2GetTireForward(tireID);
    return direction;
}

glm::vec2 CarPhysics::GetTireLateral(eCarTire tireID) const
{
    debug_assert(tireID < eCarTire_COUNT);

    box2d::vec2 direction = b2GetTireLateral(tireID);
    return direction;  
}

float CarPhysics::GetCurrentSpeed() const
{
    b2Vec2 currentForwardNormal = mPhysicsBody->GetWorldVector(box2d::ForwardVector);
    b2Vec2 forwardVelocity = b2Dot(currentForwardNormal, mPhysicsBody->GetLinearVelocity()) * currentForwardNormal;
    return b2Dot(forwardVelocity, currentForwardNormal);
}

void CarPhysics::HandleFallBegin()
{
    mFalling = true;
    mFallStartHeight = mHeight;
}

void CarPhysics::HandleFallEnd()
{
    if (mFallStartHeight > mHeight)
    {
        DamageInfo damageInfo;
        damageInfo.SetDamageFromFall(mFallStartHeight - mHeight);
        mReferenceCar->ReceiveDamage(damageInfo);
    }
    mFalling = false;
}

void CarPhysics::UpdateSteer(const DriveCtlState& currCtlState)
{
    const float LockAngleRadians = glm::radians(30.0f);
    const float TurnSpeedPerSec = glm::radians(270.0f * 1.0f);

    float turnPerTimeStep = (TurnSpeedPerSec * gTimeManager.mGameFrameDelta);
    float desiredAngle = (LockAngleRadians * currCtlState.mSteerDirection);
    float angleToTurn = b2Clamp((desiredAngle - mSteeringAngleRadians), -turnPerTimeStep, turnPerTimeStep);
    mSteeringAngleRadians = b2Clamp(mSteeringAngleRadians + angleToTurn, -LockAngleRadians, LockAngleRadians);
}

void CarPhysics::UpdateFriction(const DriveCtlState& currCtlState)
{
    b2Vec2 linearVelocityVector = mPhysicsBody->GetLinearVelocity();
    float linearSpeed = linearVelocityVector.Normalize();

    // kill lateral velocity front tire
    {
        b2Vec2 impulse = mPhysicsBody->GetMass() * 0.20f * -b2GetTireLateralVelocity(eCarTire_Front);
        mPhysicsBody->ApplyLinearImpulse(impulse, b2GetTirePos(eCarTire_Front), true);
    }

    // kill lateral velocity rear tire
    {
        b2Vec2 impulse = mPhysicsBody->GetMass() * 0.20f * -b2GetTireLateralVelocity(eCarTire_Rear);
        mPhysicsBody->ApplyLinearImpulse(impulse, b2GetTirePos(eCarTire_Rear), true);
    }

    // rolling resistance
    if (linearSpeed > 0.0f)
    {
        float rrCoef = 520.0f;
        mPhysicsBody->ApplyForce(rrCoef * -linearVelocityVector, b2GetTirePos(eCarTire_Front), true);
        mPhysicsBody->ApplyForce(rrCoef * -linearVelocityVector, b2GetTirePos(eCarTire_Rear), true);
    }

    // apply drag force
    if (linearSpeed > 0.0f)
    {
        float dragForceCoef = 102.0f;
        b2Vec2 dragForce = -dragForceCoef * linearSpeed * linearVelocityVector;

        mPhysicsBody->ApplyForceToCenter(dragForce, true);
    }
}

void CarPhysics::UpdateDrive(const DriveCtlState& currCtlState)
{
    if (currCtlState.mDriveDirection == 0.0f)
        return;

    float driveForce = 6750.0f;
    float brakeForce = driveForce * mCarDesc->mHandbrakeFriction;
    float reverseForce = driveForce * 0.75f;

    float currentSpeed = GetCurrentSpeed();
    float engineForce = 0.0f;

    if (currCtlState.mDriveDirection > 0.0f)
    {
        engineForce = driveForce;
    }
    else
    {
        if (currentSpeed > 0.0f)
        {
            engineForce = brakeForce;
        }
        else
        {
            engineForce = reverseForce;
        }
    }

    b2Vec2 F = engineForce * currCtlState.mDriveDirection * b2GetTireForward(eCarTire_Rear);
    mPhysicsBody->ApplyForce(F, b2GetTirePos(eCarTire_Rear), true);
}

b2Vec2 CarPhysics::b2GetTireLateralVelocity(eCarTire tireID) const
{
    b2Vec2 normal_vector = b2GetTireLateral(tireID);
    b2Vec2 local_position = b2GetTireLocalPos(tireID);
    return b2Dot(normal_vector, mPhysicsBody->GetLinearVelocityFromLocalPoint(local_position)) * normal_vector;
}

b2Vec2 CarPhysics::b2GetTireForwardVelocity(eCarTire tireID) const
{
    b2Vec2 normal_vector = b2GetTireForward(tireID);
    b2Vec2 local_position = b2GetTireLocalPos(tireID);
    return b2Dot(normal_vector, mPhysicsBody->GetLinearVelocityFromLocalPoint(local_position)) * normal_vector;
}

b2Vec2 CarPhysics::b2GetTireForward(eCarTire tireID) const
{
    b2Vec2 b2LocalVector = b2GetTireLocalForward(tireID);
    return mPhysicsBody->GetWorldVector(b2LocalVector);
}

b2Vec2 CarPhysics::b2GetTireLateral(eCarTire tireID) const
{
    b2Vec2 b2LocalVector = b2GetTireLocalLateral(tireID);
    return mPhysicsBody->GetWorldVector(b2LocalVector);
}

b2Vec2 CarPhysics::b2GetTirePos(eCarTire tireID) const
{
    b2Vec2 b2LocalPoint = b2GetTireLocalPos(tireID);
    return mPhysicsBody->GetWorldPoint(b2LocalPoint);
}

b2Vec2 CarPhysics::b2GetTireLocalPos(eCarTire tireID) const
{
    if (tireID == eCarTire_Rear)
    {
        return (box2d::ForwardVector * mRearTireOffset);
    }
    if (tireID == eCarTire_Front)
    {
        return (box2d::ForwardVector * mFrontTireOffset);
    }
    debug_assert(false);
    return {};
}

b2Vec2 CarPhysics::b2GetTireLocalForward(eCarTire tireID) const
{
    if (tireID == eCarTire_Rear)
    {
        return box2d::ForwardVector;
    }
    if (tireID == eCarTire_Front)
    {
        b2Rot vectorAngle (mSteeringAngleRadians);
        return b2Mul(vectorAngle, box2d::ForwardVector);
    }
    debug_assert(false);
    return {};
}

b2Vec2 CarPhysics::b2GetTireLocalLateral(eCarTire tireID) const
{
    if (tireID == eCarTire_Rear)
    {
        return box2d::LateralVector;
    }
    if (tireID == eCarTire_Front)
    {
        b2Rot vectorAngle (mSteeringAngleRadians);
        return b2Mul(vectorAngle, box2d::LateralVector);
    }
    debug_assert(false);
    return {};
}

//////////////////////////////////////////////////////////////////////////

ProjectilePhysics::ProjectilePhysics(b2World* physicsWorld, Projectile* object)
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

void ProjectilePhysics::SimulationStep()
{
    if (mReferenceProjectile->mWeaponInfo == nullptr)
    {
        debug_assert(false);
        return;
    }

    // setup physics
    glm::vec2 velocity = GetSignVector();
    SetLinearVelocity(velocity * mReferenceProjectile->mWeaponInfo->mProjectileSpeed);

    glm::vec2 currPosition = GetPosition2();
    glm::vec2 startPosition(mReferenceProjectile->mSpawnPosition.x, mReferenceProjectile->mSpawnPosition.z);

    if (glm::distance(startPosition, currPosition) >= mReferenceProjectile->mWeaponInfo->mBaseHitRange)
    {
        mReferenceProjectile->MarkForDeletion();
    }
}

bool ProjectilePhysics::ShouldContactWith(unsigned int objCatBits) const
{
    if (mContactDetected || mReferenceProjectile->IsMarkedForDeletion())
        return false;

    return true;
}

bool ProjectilePhysics::ProcessContactWithObject(const glm::vec3& contactPoint, GameObject* gameObject)
{
    if (mContactDetected || mReferenceProjectile->IsMarkedForDeletion())
        return false;

    if (mReferenceProjectile->mShooter && (mReferenceProjectile->mShooter == gameObject)) // ignore shooter ped
        return false;

    mContactPoint = contactPoint;
    mContactObject = gameObject;
    mContactDetected = true;
    return true;
}

bool ProjectilePhysics::ProcessContactWithMap(const glm::vec3& contactPoint)
{
    if (mContactDetected || mReferenceProjectile->IsMarkedForDeletion())
        return false;

    mContactPoint = contactPoint;
    mContactDetected = true;
    return true;
}

void ProjectilePhysics::ClearCurrentContact()
{
    mContactDetected = false;
    mContactObject.reset();
}
