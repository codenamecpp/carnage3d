#include "stdafx.h"
#include "FreeLookCameraController.h"

void FreeLookCameraController::SetupInitial()
{
    glm::vec3 cameraPosition = gCamera.mPosition;
    // set camera defaults
    gCamera.SetIdentity();
    gCamera.SetPerspectiveProjection(gSystem.mConfig.mScreenAspectRatio, 60.0f, 0.1f, 1000.0f);
    gCamera.SetTopDownOrientation();
    gCamera.SetPosition(cameraPosition);

    // reset controls flags
    mMoveLeft = false;
    mMoveRight = false;
    mMoveForward = false;
    mMoveBackward = false;
    mMouseDragCamera = false;
}

void FreeLookCameraController::UpdateFrame(Timespan deltaTime)
{
    if (mMoveBackward || mMoveForward || mMoveLeft || mMoveRight)
    {
        glm::vec3 moveDirection {0.0f};
        if (mMoveForward)
        {
            moveDirection -= glm::normalize(glm::cross(gCamera.mRightDirection, SceneAxes::Y));
        }
        else if (mMoveBackward)
        {
            moveDirection += glm::normalize(glm::cross(gCamera.mRightDirection, SceneAxes::Y));
        }

        if (mMoveRight)
        {
            moveDirection += gCamera.mRightDirection;
        }
        else if (mMoveLeft)
        {
            moveDirection -= gCamera.mRightDirection;
        }
        moveDirection = glm::normalize(moveDirection) * 5.0f * deltaTime.ToSeconds();
        gCamera.Translate(moveDirection);
    }

    if (mMouseDragCamera)
    {
        const float rotationAngle = 90.0f;
        if (mRotateDeltaY)
        {
            float angleRads = -glm::radians(glm::sign(mRotateDeltaY) * rotationAngle * deltaTime.ToSeconds());
            glm::vec3 frontdir = glm::rotate(gCamera.mFrontDirection, angleRads, gCamera.mRightDirection);
            glm::vec3 updir = glm::normalize(glm::cross(gCamera.mRightDirection, frontdir)); 
            gCamera.SetOrientation(frontdir, gCamera.mRightDirection, updir);
        }

        if (mRotateDeltaX)
        {
            float angleRads = -glm::radians(glm::sign(mRotateDeltaX) * rotationAngle * deltaTime.ToSeconds());
            glm::vec3 rightdir = glm::rotate(gCamera.mRightDirection, angleRads, SceneAxes::Y);
            glm::vec3 frontdir = glm::rotate(gCamera.mFrontDirection, angleRads, SceneAxes::Y);
            glm::vec3 updir = glm::normalize(glm::cross(rightdir, frontdir)); 
            gCamera.SetOrientation(frontdir, rightdir, updir);
        }   

        mRotateDeltaX = 0;
        mRotateDeltaY = 0;
    }
}

void FreeLookCameraController::InputEvent(KeyInputEvent& inputEvent)
{
    if (inputEvent.mConsumed)
        return;

    if (inputEvent.mKeycode == eKeycode_W)
    {
        mMoveForward = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == eKeycode_S)
    {
        mMoveBackward = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == eKeycode_D)
    {
        mMoveRight = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == eKeycode_A)
    {
        mMoveLeft = inputEvent.mPressed;
    }
}

void FreeLookCameraController::InputEvent(MouseButtonInputEvent& inputEvent) 
{
    if (inputEvent.mConsumed)
        return;

    if (inputEvent.mButton == eMButton_LEFT)
    {
        mMouseDragCamera = inputEvent.mPressed;
        if (mMouseDragCamera)
        {
            mLastMouseX = gInputs.mCursorPositionX;
            mLastMouseY = gInputs.mCursorPositionY;
            mRotateDeltaX = 0;
            mRotateDeltaY = 0;
        }
    }
}

void FreeLookCameraController::InputEvent(MouseMovedInputEvent& inputEvent)
{
    if (inputEvent.mConsumed)
        return;

    if (!mMouseDragCamera)
        return;

    mRotateDeltaX = inputEvent.mCursorPositionX - mLastMouseX;
    mRotateDeltaY = inputEvent.mCursorPositionY - mLastMouseY;

    mLastMouseX = inputEvent.mCursorPositionX;
    mLastMouseY = inputEvent.mCursorPositionY;
}

void FreeLookCameraController::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (inputEvent.mConsumed)
        return;

    gCamera.Translate({ 0.0f, MAP_BLOCK_LENGTH * 0.5f * -inputEvent.mScrollY, 0.0f});
}