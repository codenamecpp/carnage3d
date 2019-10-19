#pragma once

#include "GameDefs.h"

// defines body that exists in physics world and can be collided
class PhysicsComponent: public cxx::noncopyable
{
    friend class PhysicsManager;

public:    
    // public for convenience, should not be modified directly

    float mHeight; // world y coord
        // since game using 2d physics engine, the forces and impulses are not affected on 3rd dimension
        // gravity is simulated roughly

    // current state flags
    bool mOnTheGround;

public:
    // set/get object's world position and rotation angle
    // @param position: Coordinate
    // @param rotationAngle: Rotation, optional
    void SetPosition(const glm::vec3& position);
    void SetPosition(const glm::vec3& position, cxx::angle_t rotationAngle);
    glm::vec3 GetPosition() const;
    // set/get object's heading angle 
    // @param rotationAngle: Angle value
    void SetRotationAngle(cxx::angle_t rotationAngle);
    cxx::angle_t GetRotationAngle() const;
    // set/get current angular velocity
    // @param velocity: new angular velocity in degrees/second
    void SetAngularVelocity(float angularVelocity);
    float GetAngularVelocity() const;
	/// set/get the linear velocity of the center of mass
	/// @param velocity: new linear velocity of the center of mass
    void SetLinearVelocity(const glm::vec2& velocity);
    glm::vec2 GetLinearVelocity() const;
    glm::vec2 GetSignVector() const;
    // convert coordinate from local to world space and vice versa
    glm::vec2 GetWorldPoint(const glm::vec2& localPosition) const;
    glm::vec2 GetLocalPoint(const glm::vec2& worldPosition) const;
    // apply an impulse to the center of mass
    // @param impulse: The world impulse vector, usually in N-seconds or kg-m/s
    void AddLinearImpulse(const glm::vec2& impulse);
	// apply a force to the center of mass
    // @param force: Force, the world force vector, usually in Newtons (N)
    void AddForce(const glm::vec2& force);
	// apply an angular impulse
	// @param impulse the angular impulse in units of kg*m*m/s
    void AddAngularImpulse(float impulse);
    // cancel currently active forces
    void ClearForces();

    // attach or detach physics component to car
    // @param component: Component to attach
    // @param localAnchor: Attachment point local coordinate
    void AttachComponent(PhysicsComponent* component, const glm::vec2& localAnchor);
    void DetachComponent(PhysicsComponent* component);
    void DetachAllComponents();

protected:
    // only derived classes could be instantiated
    PhysicsComponent(b2World* physicsWorld);
    virtual ~PhysicsComponent();

protected:
    // box2d specific objects is could be accessed only by derived classes and physics manager himself
    struct PhysicsConnection
    {
        b2Joint* mJoint;
        PhysicsComponent* mPhysicsComponent;
    };

    b2World* mPhysicsWorld;
    b2Body* mPhysicsBody;
    std::vector<PhysicsConnection> mConnections;
};

// pedestrian physics component
class PedPhysicsComponent: public PhysicsComponent
{
public:
    // @param startPosition: Initial world position
    // @param startRotation: Initial rotation
    PedPhysicsComponent(b2World* physicsWorld, const glm::vec3& startPosition, cxx::angle_t startRotation);
    ~PedPhysicsComponent();

    void UpdateFrame(Timespan deltaTime);
    void SetFalling(bool isFalling);
    void HandleCarContactBegin();
    void HandleCarContactEnd();
    void HandleCarEnter();
    void HandleCarLeave();
    // test whether pedestrian should collide with other objects depending on its current state
    // @param objCatBits: object categories bits see PHYSICS_OBJCAT_* bits
    bool ShouldCollideWith(unsigned int objCatBits) const;

public:
    Pedestrian* mReferencePed = nullptr;
    int mContactingCars = 0; // number of contacting cars
    bool mFalling = false; // falling from a height
};

enum eCarWheelID
{
    eCarWheelID_Steering,
    eCarWheelID_Drive,
    eCarWheelID_COUNT
};

// car chassis physics component
class CarPhysicsComponent: public PhysicsComponent
{
public:
    Vehicle* mReferenceCar = nullptr;

public:
    // @param desc: Car style description, cannot be null
    // @param startPosition: Initial world position
    // @param startRotation: Initial rotation
    CarPhysicsComponent(b2World* physicsWorld, CarStyle* desc, const glm::vec3& startPosition, cxx::angle_t startRotation);
    ~CarPhysicsComponent();

    void UpdateFrame(Timespan deltaTime);
    void GetChassisCorners(glm::vec2 corners[4]) const;
    void GetWheelCorners(eCarWheelID wheelID, glm::vec2 corners[4]) const;
    // test whether specific wheel exists
    bool HasWheel(eCarWheelID wheelID) const;
    
    glm::vec2 GetWheelLateralVelocity(eCarWheelID wheelID) const;
    glm::vec2 GetWheelForwardVelocity(eCarWheelID wheelID) const;
    glm::vec2 GetWheelPosition(eCarWheelID wheelID) const;

private:

    // car internals wheel data
    struct WheelData
    {
    public:
        WheelData() = default;
    public:
        b2Body* mPhysicsBody = nullptr;
        b2Fixture* mFixture = nullptr;
    };

    void SetupWheels(CarStyle* desc);
    void FreeWheels();
    void CreateWheel(CarStyle* desc, eCarWheelID wheelID);

    b2Vec2 GetWheelLateralVelocity(b2Body* carWheel) const;
    b2Vec2 GetWheelForwardVelocity(b2Body* carWheel) const;

private:
    b2Fixture* mChassisFixture = nullptr;
    b2RevoluteJoint* mFrontWheelJoint = nullptr;
    b2RevoluteJoint* mRearWheelJoint = nullptr;
    WheelData mCarWheels[eCarWheelID_COUNT];
};