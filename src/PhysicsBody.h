#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"

// defines body that exists in physics world and can be collided
// it contains at least one physics shape
class PhysicsBody final: public cxx::noncopyable
{
    friend class PhysicsManager;
    friend class Collider;

public:    
    // readonly
    GameObject* mGameObject = nullptr; // owner game object
    std::vector<Collider*> mColliders;

    float mPositionY; // world y coord
        // since game using 2d physics engine, the forces and impulses are not affected on 3rd dimension
        // gravity is simulated roughly

    bool mWaterContact = false; // fall into water
    bool mFalling = false; // falling from a height
    float mFallStartHeight = 0.0f; // specified if mFalling is set

public:
    PhysicsBody(GameObject* owner, PhysicsBodyFlags flags);
    ~PhysicsBody();

    // Setup body flags
    void SetupFlags(PhysicsBodyFlags flags);
    bool CheckFlags(PhysicsBodyFlags flags) const;
    void ChangeFlags(PhysicsBodyFlags enableFlags, PhysicsBodyFlags disableFlags);

    // Create or delete collision shape for body
    Collider* AddCollider(int colliderIndex, const CollisionShape& shapeData, const PhysicsMaterial& shapeMaterial,
        CollisionGroup collisionGroup,
        CollisionGroup collidesWith, ColliderFlags colliderFlags);
    
    bool DeleteCollider(int colliderIndex);
    bool DeleteCollider(Collider* collider);
    void DeleteColliders();

    Collider* GetColliderWithIndex(int colliderIndex) const;
    bool HasColliderWithIndex(int colliderIndex) const;

    // Set or get object's world position and rotation angle
    // @param position: Coordinate, meters
    // @param rotationAngle: Angle
    void SetTransform(const glm::vec3& position, cxx::angle_t rotationAngle);
    void SetTransform(const glm::vec3& position);
    void SetOrientation(cxx::angle_t rotationAngle);
    glm::vec3 GetPosition() const;
    glm::vec2 GetPosition2() const;
    cxx::angle_t GetOrientation() const;

    // Set sign direction
    // @param signDirection: Sign direction vector, normalized
    void SetSignVector(const glm::vec2& signDirection);
    glm::vec2 GetSignVector() const;

    // Set or get current angular velocity
    // @param velocity: New angular velocity angle per second
    void SetAngularVelocity(cxx::angle_t angularVelocity);
    cxx::angle_t GetAngularVelocity() const;

	/// Set or get the linear velocity of the center of mass
	/// @param velocity: New linear velocity of the center of mass, meters per second
    void SetLinearVelocity(const glm::vec2& velocity);
    glm::vec2 GetLinearVelocity() const;
    glm::vec2 GetLinearVelocityFromWorldPoint(const glm::vec2& worldPosition) const;
    glm::vec2 GetLinearVelocityFromLocalPoint(const glm::vec2& localPosition) const;

    float GetMass() const;

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
    void ApplyLinearImpulse(const glm::vec2& impulse);
    void ApplyLinearImpulse(const glm::vec2& impulse, const glm::vec2& position);

	// Apply a force to the center of mass
    // @param force: Force, the world force vector, usually in Newtons (N)
    // @param position: World position, meters
    void AddForce(const glm::vec2& force);
    void AddForce(const glm::vec2& force, const glm::vec2& position);

	// Apply an angular impulse
	// @param impulse the angular impulse in units of kg*m*m/s
    void ApplyAngularImpulse(float impulse);

    // Cancel currently active forces
    void ClearForces();

    // Whether body is'nt sleeping
    void SetAwake(bool isAwake);
    bool IsAwake() const;

private:
    PhysicsBodyFlags mBodyFlags = PhysicsBodyFlags_None;
    b2Body* mBox2Body = nullptr;
};
