#pragma once

#include "GameDefs.h"

// defines body that exists in physics world and can be collided
class PhysicsBody: public cxx::noncopyable
{
    friend class PhysicsManager;

    friend class PedPhysicsBody;
    friend class CarPhysicsBody;
    friend class ProjectilePhysicsBody;

public:    
    // readonly

    float mHeight; // world y coord
        // since game using 2d physics engine, the forces and impulses are not affected on 3rd dimension
        // gravity is simulated roughly

    bool mWaterContact = false; // fall into water
    bool mFalling = false; // falling from a height
    float mFallStartHeight = 0.0f; // specified if mFalling is set

    // for rendering
    glm::vec3 mPreviousPosition;
    glm::vec3 mSmoothPosition; 
    cxx::angle_t mPreviousRotation;
    cxx::angle_t mSmoothRotation;

public:
    virtual ~PhysicsBody();

    virtual void SimulationStep() = 0;

    // test whether object should contact with other objects depending on its current state
    // @param objCatBits: object categories bits see PHYSICS_OBJCAT_* bits
    virtual bool ShouldContactWith(unsigned int objCatBits) const = 0;

    // Set or get object's world position and rotation angle
    // @param position: Coordinate, meters
    // @param rotationAngle: Angle
    void SetPosition(const glm::vec3& position);
    void SetPosition(const glm::vec3& position, cxx::angle_t rotationAngle);
    glm::vec3 GetPosition() const;
    glm::vec2 GetPosition2() const;

    // Set or get object's heading angle 
    // @param rotationAngle: Angle
    void SetRotationAngle(cxx::angle_t rotationAngle);
    cxx::angle_t GetRotationAngle() const;

    // Set sign direction
    // @param signDirection: Sign direction vector, normalized
    void SetOrientation2(const glm::vec2& signDirection);
    glm::vec2 GetSignVector() const;

    // Set or get current angular velocity
    // @param velocity: New angular velocity in degrees/second
    void SetAngularVelocity(cxx::angle_t angularVelocity);
    cxx::angle_t GetAngularVelocity() const;

	/// Set or get the linear velocity of the center of mass
	/// @param velocity: New linear velocity of the center of mass, meters per second
    void SetLinearVelocity(const glm::vec2& velocity);
    glm::vec2 GetLinearVelocity() const;
    glm::vec2 GetLinearVelocityFromWorldPoint(const glm::vec2& worldPosition) const;
    glm::vec2 GetLinearVelocityFromLocalPoint(const glm::vec2& localPosition) const;

    // Convert coordinate from local to world space and vice versa
    // @param localPosition, worldPosition: Coordinate in meters
    glm::vec2 GetWorldPoint(const glm::vec2& localPosition) const;
    glm::vec2 GetLocalPoint(const glm::vec2& worldPosition) const;

    // Convert vector from local to world space and vice versa
    glm::vec2 GetWorldVector(const glm::vec2& localVector) const;
    glm::vec2 GetLocalVector(const glm::vec2& worldVector) const;
    
    // Apply an impulse to the center of mass
    // @param impulse: The world impulse vector, usually in N-seconds or kg-m/s
    // @param position: World position, meters
    void AddLinearImpulse(const glm::vec2& impulse);
    void AddLinearImpulse(const glm::vec2& impulse, const glm::vec2& position);

	// Apply a force to the center of mass
    // @param force: Force, the world force vector, usually in Newtons (N)
    // @param position: World position, meters
    void AddForce(const glm::vec2& force);
    void AddForce(const glm::vec2& force, const glm::vec2& position);

	// Apply an angular impulse
	// @param impulse the angular impulse in units of kg*m*m/s
    void AddAngularImpulse(float impulse);

    // Cancel currently active forces
    void ClearForces();

    // Clear state
    void SetRespawned();

protected:
    // only derived classes could be instantiated
    PhysicsBody(b2World* physicsWorld);

protected:
    b2World* mPhysicsWorld;
    b2Body* mPhysicsBody;
};

//////////////////////////////////////////////////////////////////////////

// pedestrian physics component
class PedPhysicsBody: public PhysicsBody
{
    friend class PhysicsManager;

public:
    // readonly
    Pedestrian* mReferencePed = nullptr;

    glm::vec2 mCarPointLocal; // when driving car, pedestrian body will be attached to that point
    int mContactingCars = 0; // number of contacting cars

public:
    PedPhysicsBody(b2World* physicsWorld, Pedestrian* object);
    // override PhysicsComponent
    void SimulationStep() override;
    bool ShouldContactWith(unsigned int objCatBits) const override;

    void HandleFallBegin();
    void HandleFallEnd();
    void HandleCarContactBegin();
    void HandleCarContactEnd();
    void HandleWaterContact();
};

//////////////////////////////////////////////////////////////////////////

enum eCarTire
{
    eCarTire_Front,
    eCarTire_Rear,
    eCarTire_COUNT
};

// car chassis physics component
class CarPhysicsBody: public PhysicsBody
{
    friend class PhysicsManager;

public:
    // readonly
    Vehicle* mReferenceCar = nullptr;

public:
    CarPhysicsBody(b2World* physicsWorld, Vehicle* object);
    // override PhysicsComponent
    void SimulationStep() override;
    bool ShouldContactWith(unsigned int objCatBits) const override;

    void HandleWaterContact();

    void GetChassisCorners(glm::vec2 corners[4]) const;
    void GetLocalChassisCorners(glm::vec2 corners[4]) const;
    void GetTireCorners(eCarTire tireID, glm::vec2 corners[4]) const;
    
    // Get tire velocities
    glm::vec2 GetTireLateralVelocity(eCarTire tireID) const;
    glm::vec2 GetTireForwardVelocity(eCarTire tireID) const;

    // Get tire forward direction and position in world space
    glm::vec2 GetTirePosition(eCarTire tireID) const;
    glm::vec2 GetTireForward(eCarTire tireID) const;
    glm::vec2 GetTireLateral(eCarTire tireID) const;

    // Get current vehicle speed
    float GetCurrentSpeed() const;

    void HandleFallBegin();
    void HandleFallEnd();

private:
    struct DriveCtlState
    {
        float mSteerDirection = 0.0f;
        float mDriveDirection = 0.0f;
        bool mHandBrake = false;
    };

    void UpdateSteer(const DriveCtlState& currCtlState);
    void UpdateFriction(const DriveCtlState& currCtlState);
    void UpdateDrive(const DriveCtlState& currCtlState);

    // helpers, world space
    b2Vec2 b2GetTireLateralVelocity(eCarTire tireID) const;
    b2Vec2 b2GetTireForwardVelocity(eCarTire tireID) const;
    b2Vec2 b2GetTireForward(eCarTire tireID) const;
    b2Vec2 b2GetTireLateral(eCarTire tireID) const;
    b2Vec2 b2GetTirePos(eCarTire tireID) const;
    // helpers, local space
    b2Vec2 b2GetTireLocalPos(eCarTire tireID) const;
    b2Vec2 b2GetTireLocalForward(eCarTire tireID) const;
    b2Vec2 b2GetTireLocalLateral(eCarTire tireID) const;

private:
    VehicleInfo* mCarDesc = nullptr;
    b2Fixture* mChassisFixture = nullptr;

    // tires
    float mFrontTireOffset = 0.0f; // steer
    float mRearTireOffset = 0.0f; // drive
    float mSteeringAngleRadians = 0.0f;
};

//////////////////////////////////////////////////////////////////////////

class ProjectilePhysicsBody: public PhysicsBody
{
    friend class PhysicsManager;

public:
    // readonly
    Projectile* mReferenceProjectile = nullptr;

public:
    ProjectilePhysicsBody(b2World* physicsWorld, Projectile* object);
    // override PhysicsComponent
    void SimulationStep() override;
    bool ShouldContactWith(unsigned int objCatBits) const override;
};
