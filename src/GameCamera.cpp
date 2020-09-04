#include "stdafx.h"
#include "GameCamera.h"

GameCamera::GameCamera()
    : mProjMatrixDirty(true)
    , mViewMatrixDirty(true)
    , mCurrentMode(eSceneCameraMode_Perspective)
    , mViewportRect()
{
    SetIdentity();
}

void GameCamera::SetPerspectiveProjection(float aspect, float fovy, float nearPlane, float farPlane)
{
    mCurrentMode = eSceneCameraMode_Perspective;
    mPerspectiveParams.mAspect = aspect;
    mPerspectiveParams.mFovy = fovy;
    mPerspectiveParams.mNearPlane = nearPlane;
    mPerspectiveParams.mFarPlane = farPlane;
    mProjMatrixDirty = true;
}

void GameCamera::SetOrthographicProjection(float leftp, float rightp, float bottomp, float topp)
{
    mCurrentMode = eSceneCameraMode_Orthographic;
    mOrthographicParams.mLeftP = leftp;
    mOrthographicParams.mRightP = rightp;
    mOrthographicParams.mBottomP = bottomp;
    mOrthographicParams.mTopP = topp;
    mProjMatrixDirty = true;
}

void GameCamera::ComputeMatricesAndFrustum()
{
    bool computeViewProjectionMatrix = mProjMatrixDirty || mViewMatrixDirty;
    if (mProjMatrixDirty)
    {
        if (mCurrentMode == eSceneCameraMode_Perspective)
        {
            mProjectionMatrix = glm::perspective(glm::radians(mPerspectiveParams.mFovy), 
                mPerspectiveParams.mAspect, mPerspectiveParams.mNearPlane, mPerspectiveParams.mFarPlane);
        }
        else
        {
            mProjectionMatrix = glm::ortho(mOrthographicParams.mLeftP, mOrthographicParams.mRightP, 
                mOrthographicParams.mBottomP, mOrthographicParams.mTopP);
        }
        mProjMatrixDirty = false;
    }

    if (mViewMatrixDirty)
    {
        mViewMatrix = glm::lookAt(mPosition, mPosition + mFrontDirection, mUpDirection);
        mViewMatrixDirty = false;
    }

    if (computeViewProjectionMatrix)
    {
        mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;

        // recompute frustum planes
        mFrustum.compute_from_viewproj_matrix(mViewProjectionMatrix);
    }
}

void GameCamera::SetIdentity()
{
    // set ident matrices
    mProjectionMatrix = glm::mat4(1.0f);
    mViewProjectionMatrix = glm::mat4(1.0f);
    mViewMatrix = glm::mat4(1.0f);

    // set default axes
    mUpDirection = SceneAxisY;
    mFrontDirection = -SceneAxisZ;
    mRightDirection = SceneAxisX;

    // reset position to origin
    mPosition = glm::vec3(0.0f);

    // force dirty flags
    mProjMatrixDirty = true;
    mViewMatrixDirty = true;
}

void GameCamera::FocusAt(const glm::vec3& point, const glm::vec3& upward)
{
    mFrontDirection = glm::normalize(point - mPosition);
    mRightDirection = glm::normalize(glm::cross(upward, mFrontDirection));
    mUpDirection = glm::normalize(glm::cross(mFrontDirection, mRightDirection));
    mViewMatrixDirty = true;
}

void GameCamera::SetPosition(const glm::vec3& position)
{
    mPosition = position;
    mViewMatrixDirty = true;
}

void GameCamera::SetRotationAngles(const glm::vec3& rotationAngles)
{
    const glm::mat4 rotationMatrix = glm::eulerAngleYXZ(
        glm::radians(rotationAngles.y), 
        glm::radians(rotationAngles.x), 
        glm::radians(rotationAngles.z));

    const glm::vec3 rotatedUp = glm::vec3(rotationMatrix * glm::vec4(SceneAxisY, 0.0f));
    mFrontDirection = glm::vec3(rotationMatrix * glm::vec4(-SceneAxisZ, 0.0f));
    mRightDirection = glm::normalize(glm::cross(rotatedUp, mFrontDirection)); 
    mUpDirection = glm::normalize(glm::cross(mFrontDirection, mRightDirection));
    mViewMatrixDirty = true;
}

void GameCamera::Translate(const glm::vec3& direction)
{
    mPosition += direction;
    mViewMatrixDirty = true;
}

bool GameCamera::CastRayFromScreenPoint(const glm::ivec2& screenCoordinate, cxx::ray3d_t& resultRay)
{
    // wrap y
    const int32_t mouseY = mViewportRect.h - screenCoordinate.y;
    const glm::ivec4 viewport { 
        mViewportRect.x, 
        mViewportRect.y, 
        mViewportRect.w, 
        mViewportRect.h };
        //unproject twice to build a ray from near to far plane
    const glm::vec3 v0 = glm::unProject(glm::vec3{screenCoordinate.x * 1.0f, mouseY * 1.0f, 0.0f}, 
        mViewMatrix, 
        mProjectionMatrix, viewport); // near plane

    const glm::vec3 v1 = glm::unProject(glm::vec3{screenCoordinate.x * 1.0f, mouseY * 1.0f, 1.0f}, 
        mViewMatrix, 
        mProjectionMatrix, viewport); // far plane

    resultRay.mOrigin = v0;
    resultRay.mDirection = glm::normalize(v1 - v0);
    return true;
}

void GameCamera::SetTopDownOrientation()
{
    mFrontDirection = -SceneAxisY;
    mUpDirection = -SceneAxisZ;
    mRightDirection = SceneAxisX;
    mViewMatrixDirty = true;
}

void GameCamera::SetOrientation(const glm::vec3& dirForward, const glm::vec3& dirRight, const glm::vec3& dirUp)
{
    mFrontDirection = dirForward;
    mUpDirection = dirUp;
    mRightDirection = dirRight;
    mViewMatrixDirty = true;
}

cxx::aabbox2d_t GameCamera::ComputeViewBounds2() const
{
    cxx::aabbox2d_t areaBounds;
    if (mCurrentMode == eSceneCameraMode_Perspective)
    {
        const float PiDiv180 = 0.017453f;
        const float Hfar = tan(mPerspectiveParams.mFovy * PiDiv180 * 0.5f) * mPosition.y;
        const float Wfar = Hfar * mPerspectiveParams.mAspect;

        areaBounds.mMin.x = mPosition.x - Wfar;
        areaBounds.mMin.y = mPosition.z - Hfar;
        areaBounds.mMax.x = mPosition.x + Wfar;
        areaBounds.mMax.y = mPosition.z + Hfar;
    }

    return areaBounds;
}

//////////////////////////////////////////////////////////////////////////

GameCamera2D::GameCamera2D(): mViewportRect()
{
    SetIdentity();
}

void GameCamera2D::SetProjection(float leftp, float rightp, float bottomp, float topp)
{
    mProjectionMatrix = glm::ortho(leftp, rightp, bottomp, topp);
}

void GameCamera2D::SetIdentity()
{
    mProjectionMatrix = glm::mat4(1.0f);
}
