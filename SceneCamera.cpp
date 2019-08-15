#include "stdafx.h"
#include "SceneCamera.h"

SceneCamera gCamera;

SceneCamera::SceneCamera()
    : mProjMatrixDirty(true)
    , mViewMatrixDirty(true)
    , mCurrentMode(eSceneCameraMode_Perspective)
{
    SetIdentity();
}

void SceneCamera::SetPerspectiveProjection(float aspect, float fovy, float nearPlane, float farPlane)
{
    mCurrentMode = eSceneCameraMode_Perspective;
    mPerspectiveParams.mAspect = aspect;
    mPerspectiveParams.mFovy = fovy;
    mPerspectiveParams.mNearPlane = nearPlane;
    mPerspectiveParams.mFarPlane = farPlane;
    mProjMatrixDirty = true;
}

void SceneCamera::SetOrthographicProjection(float leftp, float rightp, float bottomp, float topp)
{
    mCurrentMode = eSceneCameraMode_Orthographic;
    mOrthographicParams.mLeftP = leftp;
    mOrthographicParams.mRightP = rightp;
    mOrthographicParams.mBottomP = bottomp;
    mOrthographicParams.mTopP = topp;
    mProjMatrixDirty = true;
}

void SceneCamera::ComputeMatricesAndFrustum()
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

void SceneCamera::SetIdentity()
{
    // set ident matrices
    mProjectionMatrix = glm::mat4(1.0f);
    mViewProjectionMatrix = glm::mat4(1.0f);
    mViewMatrix = glm::mat4(1.0f);

    // set default axes
    mUpDirection = SceneAxes::Y;
    mFrontDirection = -SceneAxes::Z;
    mRightDirection = SceneAxes::X;

    // reset position to origin
    mPosition = glm::vec3(0.0f);

    // force dirty flags
    mProjMatrixDirty = true;
    mViewMatrixDirty = true;
}

void SceneCamera::FocusAt(const glm::vec3& point, const glm::vec3& upward)
{
    mFrontDirection = glm::normalize(point - mPosition);
    mRightDirection = glm::normalize(glm::cross(upward, mFrontDirection));
    mUpDirection = glm::normalize(glm::cross(mFrontDirection, mRightDirection));
    mViewMatrixDirty = true;
}

void SceneCamera::SetPosition(const glm::vec3& position)
{
    mPosition = position;
    mViewMatrixDirty = true;
}

void SceneCamera::SetRotationAngles(const glm::vec3& rotationAngles)
{
    const glm::mat4 rotationMatrix = glm::eulerAngleYXZ(
        glm::radians(rotationAngles.y), 
        glm::radians(rotationAngles.x), 
        glm::radians(rotationAngles.z));

    const glm::vec3 rotatedUp = glm::vec3(rotationMatrix * glm::vec4(SceneAxes::Y, 0.0f));
    mFrontDirection = glm::vec3(rotationMatrix * glm::vec4(-SceneAxes::Z, 0.0f));
    mRightDirection = glm::normalize(glm::cross(rotatedUp, mFrontDirection)); 
    mUpDirection = glm::normalize(glm::cross(mFrontDirection, mRightDirection));
    mViewMatrixDirty = true;
}

void SceneCamera::Translate(const glm::vec3& direction)
{
    mPosition += direction;
    mViewMatrixDirty = true;
}
