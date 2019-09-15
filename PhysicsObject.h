#pragma once

// defines object that exists in physics world and can be collided
class PhysicsObject final: public cxx::noncopyable
{
public:    
    // public for convenience, should not be modified directly
    float mZCoord;
    float mDepth; // height in z

    bool mFalling;
    bool mGhost; // ignore collisions
    bool mInCollidingWithWall;

public:
    PhysicsObject();
    ~PhysicsObject();

    // set/get object's world position and rotation angle
    // @param position: Coordinate
    // @param angleDegrees: Rotation, optional
    void SetPosition(const glm::vec3& position);
    void SetPosition(const glm::vec3& position, float angleDegrees);
    glm::vec3 GetPosition() const;

    // set object's heading angle in degrees or radians
    // @param angleDegrees, angleRadians: Angle
    void SetAngleDegrees(float angleDegrees);
    void SetAngleRadians(float angleRadians);

    // set/get current angular velocity
    // @param velocity: new angular velocity in degrees/second
    void SetAngularVelocity(float angularVelocity);
    float GetAngularVelocity() const;

	/// set/get the linear velocity of the center of mass
	/// @param velocity: new linear velocity of the center of mass
    void SetLinearVelocity(const glm::vec3& velocity);
    glm::vec3 GetLinearVelocity() const;

	// apply a force to the center of mass
    // @param force: Force, the world force vector, usually in Newtons (N)
    void ApplyForce(const glm::vec3& force);

    // apply an impulse to the center of mass
    // @param impulse: The world impulse vector, usually in N-seconds or kg-m/s
    void ApplyLinearImpulse(const glm::vec3& impulse);

	// apply an angular impulse
	// @param impulse the angular impulse in units of kg*m*m/s
    void ApplyAngularImpulse(float impulse);

    // get current rotation angle in degrees or radians
    float GetAngleDegrees() const;
    float GetAngleRadians() const;

private:
    friend class PhysicsManager;
    // internal stuff that should be touched only by physics manager
    b2World* mPhysicsWorld;
    b2Body* mPhysicsBody;
};