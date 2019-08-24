#pragma once

#include "CommonTypes.h"

// defines camera in 3d space
class GameCamera final: public cxx::noncopyable
{
public:
    // public for convenience, should not be modified directly
    glm::vec3 mPosition;
    glm::vec3 mFrontDirection;
    glm::vec3 mUpDirection;
    glm::vec3 mRightDirection;
    // before reading those matrices make sure to ComputeMatricesAndFrustum
    // should not be modified directly
    cxx::frustum_t mFrustum;
    glm::mat4 mViewMatrix; 
    glm::mat4 mViewProjectionMatrix;
    glm::mat4 mProjectionMatrix;

    eSceneCameraMode mCurrentMode;

public:
    GameCamera();

    // Setup perspective projection matrix
    // @param aspect: Screen aspect ratio
    // @param fovy: Field of view in degrees
    // @param nearPlane: Near clipping plane distance
    // @param farPlane: Far clipping plane distance
    void SetPerspectiveProjection(float aspect, float fovy, float nearPlane, float farPlane);

    // Setup orthographic projection matrix
    // @param leftp, rightp, bottomp, topp
    void SetOrthographicProjection(float leftp, float rightp, float bottomp, float topp);

    // Refresh view and projection matrices along with camera frustum
    // Will not do any unnecessary calculations if nothing changed
    void ComputeMatricesAndFrustum();

    // Reset camera to initial state
    void SetIdentity();

    // Setup camera orientation, look at specified point
    // @param point: Point world
    // @param upward: Up vector, should be normalized
    void FocusAt(const glm::vec3& point, const glm::vec3& upward);

    // Set camera position
    // @param position: Camera new position
    void SetPosition(const glm::vec3& position);

    // Set camera rotation
    // @param rotationAngles: Angles in degrees
    void SetRotationAngles(const glm::vec3& rotationAngles);

    // Move camera position
    // @param direction: Move direction
    void Translate(const glm::vec3& direction);

    // Cast ray in specific viewport coordinate, make sure to ComputeMatricesAndFrustum
    // @param screenCoordinate: 2d coordinates on screen
    // @param resultRay: Output ray info
    bool CastRayFromScreenPoint(const glm::ivec2& screenCoordinate, cxx::ray3d_t& resultRay);

private:
    bool mProjMatrixDirty; // projection matrix need recomputation
    bool mViewMatrixDirty; // view matrix need recomputation

    // projection parameters

    struct PerspectiveParams
    {
        float mAspect = 1.0f;
        float mFovy = 1.0f;
        float mNearPlane = 1.0;
        float mFarPlane = 1.0;
    };
    PerspectiveParams mPerspectiveParams;

    struct OrthographicParams
    {
        float mLeftP = 1.0f;
        float mRightP = 1.0;
        float mBottomP = 1.0;
        float mTopP = 1.0;
    };
    OrthographicParams mOrthographicParams;
};

extern GameCamera gCamera;