#pragma once

//////////////////////////////////////////////////////////////////////////

enum eTransformSpace
{
    eTransformSpace_Local, // relative to parent
    eTransformSpace_World, // absolute
};

//////////////////////////////////////////////////////////////////////////

static const glm::vec2 LocalForwardVector (1.0f, 0.0f);
static const glm::vec2 LocalLateralVector (0.0f, 1.0f);

//////////////////////////////////////////////////////////////////////////

class Transform
{
public:
    Transform() = default;
    Transform(const glm::vec3& transformPosition, cxx::angle_t transformAngle)
        : mPosition(transformPosition)
        , mOrientation(transformAngle)
    {
    }

    // @param directionVector: Direction vector, should be normalized
    inline void SetOrientation(const glm::vec2& signDirection)
    {
        float rotationAngleRadians = atan2f(signDirection.y, signDirection.x);
        mOrientation.set_angle(rotationAngleRadians, cxx::angle_t::units::radians);
    }

    void SetIdentity()
    {
        mPosition.x = 0.0f;
        mPosition.y = 0.0f;
        mPosition.z = 0.0f;
        mOrientation.set_zero();
    }

    inline glm::vec2 GetDirectionVector() const
    {
        float rotationAngleRadians = mOrientation.to_radians();
        return glm::vec2{
            cos(rotationAngleRadians), 
            sin(rotationAngleRadians) 
        };
    }

    inline glm::vec2 GetRightVector() const
    {
        glm::vec2 directionVector = GetDirectionVector();
        return glm::vec2(-directionVector.y, directionVector.x);
    }

    // @param transformSpace: Target space transformation
    inline glm::vec2 GetPoint2(const glm::vec2& point2, eTransformSpace transformSpace)
    {
        float sin_, cos_;
        mOrientation.get_sin_cos(sin_, cos_);

        float x;
        float z;

        if (transformSpace == eTransformSpace_Local) // global space -> local space
        {
            float px = point2.x - mPosition.x;
            float py = point2.y - mPosition.z;

            x = ( cos_ * px + sin_ * py);
            z = (-sin_ * px + cos_ * py);
        }
        else // local space -> global space
        {
            x = (cos_ * point2.x - sin_ * point2.y) + mPosition.x;
            z = (sin_ * point2.x + cos_ * point2.y) + mPosition.z;
        }
        return glm::vec2(x, z);
    }

    inline glm::vec3 GetPoint(const glm::vec3& point, eTransformSpace transformSpace)
    {
        float sin_, cos_;
        mOrientation.get_sin_cos(sin_, cos_);

        float x;
        float y;
        float z;

        if (transformSpace == eTransformSpace_Local) // global space -> local space
        {
            float px = point.x - mPosition.x;
            float py = point.z - mPosition.z;

            x = ( cos_ * px + sin_ * py);
            z = (-sin_ * px + cos_ * py);
            y = point.y - mPosition.y;
        }
        else // local space -> global space
        {
            x = (cos_ * point.x - sin_ * point.z) + mPosition.x;
            z = (sin_ * point.x + cos_ * point.z) + mPosition.z;
            y = point.y + mPosition.y;
        }
        return glm::vec3(x, y, z);
    }

    // get object position on ground plane, without height
    inline glm::vec2 GetPosition2() const
    {
        return glm::vec2{ mPosition.x, mPosition.z };
    }

    // equality operators
    inline bool operator == (const Transform& other) const
    {
        return (mPosition == other.mPosition) && (mOrientation == other.mOrientation);
    }

    inline bool operator != (const Transform& other) const
    {
        return (mPosition != other.mPosition) || (mOrientation != other.mOrientation);
    }

    inline Transform operator + (const Transform& other) const
    {
        return Transform( mPosition + other.mPosition, mOrientation + other.mOrientation );
    }

    inline Transform operator - (const Transform& other) const
    {
        return Transform( mPosition - other.mPosition, mOrientation - other.mOrientation ); 
    }

public:
    // object position within physical world
    // x and z coordinates for ground plane
    // y coordinate for height
    glm::vec3 mPosition;

    // object orientation in physical world
    cxx::angle_t mOrientation;
};

// get interpolated transform
inline Transform InterpolateTransform(const Transform& lhs, const Transform& rhs, float factor)
{
    return Transform
    {
        glm::lerp(lhs.mPosition, rhs.mPosition, factor),
        cxx::lerp_angles(lhs.mOrientation, rhs.mOrientation, factor)
    };
}