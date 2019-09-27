#pragma once

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
    // fixed frame simulation step
    virtual void SimulationStep()
    {
        // does nothing in base implementation
    }

    // set/get object's world position and rotation angle
    // @param position: Coordinate
    // @param angleDegrees: Rotation, optional
    void SetPosition(const glm::vec3& position);
    void SetPosition(const glm::vec3& position, float angleDegrees);
    glm::vec3 GetPosition() const;
    // set/get object's heading angle in degrees or radians
    // @param angleDegrees, angleRadians: Angle
    void SetAngleDegrees(float angleDegrees);
    float GetAngleDegrees() const;
    void SetAngleRadians(float angleRadians);
    float GetAngleRadians() const;
    // set/get current angular velocity
    // @param velocity: new angular velocity in degrees/second
    void SetAngularVelocity(float angularVelocity);
    float GetAngularVelocity() const;
	/// set/get the linear velocity of the center of mass
	/// @param velocity: new linear velocity of the center of mass
    void SetLinearVelocity(const glm::vec2& velocity);
    glm::vec2 GetLinearVelocity() const;
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

protected:
    // only derived classes could be instantiated
    PhysicsComponent(b2World* physicsWorld);
    virtual ~PhysicsComponent()
    {
    }

protected:
    // box2d specific objects is could be accessed only by derived classes and physics manager himself
    b2World* mPhysicsWorld;
    b2Body* mPhysicsBody;
};

// pedestrian physics component
class PedPhysicsComponent: public PhysicsComponent
{
public:
    PedPhysicsComponent(b2World* physicsWorld);
    ~PedPhysicsComponent();
};

// car wheel physics component
class WheelPhysicsComponent: public PhysicsComponent
{
public:
    WheelPhysicsComponent(b2World* physicsWorld);
    ~WheelPhysicsComponent();
};

// car chassis physics component
class CarPhysicsComponent: public PhysicsComponent
{
public:
    CarPhysicsComponent(b2World* physicsWorld, CarStyle* desc);
    ~CarPhysicsComponent();
};