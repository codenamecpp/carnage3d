#include "stdafx.h"
#include "FreeLookCameraController.h"
#include "TimeManager.h"

void FreeLookCameraController::Setup(GameCamera* gameCamera)
{
    debug_assert(gameCamera);
    mCamera = gameCamera;

    // compute aspect ratio
    float screenAspect = (mCamera->mViewportRect.h > 0) ? ((mCamera->mViewportRect.w * 1.0f) / (mCamera->mViewportRect.h * 1.0f)) : 1.0f;

    glm::vec3 cameraPosition = mCamera->mPosition;
    // set camera defaults
    mCamera->SetIdentity();
    mCamera->SetPerspectiveProjection(screenAspect, 60.0f, 0.1f, 1000.0f);
    mCamera->SetTopDownOrientation();
    mCamera->SetPosition(cameraPosition);

    // reset controls flags
    mMoveLeft = false;
    mMoveRight = false;
    mMoveForward = false;
    mMoveBackward = false;
    mMouseDragCamera = false;
}

void FreeLookCameraController::UpdateFrame()
{
    float deltaTime = gTimeManager.mSystemFrameDelta;

    if (mMoveBackward || mMoveForward || mMoveLeft || mMoveRight)
    {
        glm::vec3 moveDirection {0.0f};
        if (mMoveForward)
        {
            moveDirection -= glm::normalize(glm::cross(mCamera->mRightDirection, SceneAxisY));
        }
        else if (mMoveBackward)
        {
            moveDirection += glm::normalize(glm::cross(mCamera->mRightDirection, SceneAxisY));
        }

        if (mMoveRight)
        {
            moveDirection += mCamera->mRightDirection;
        }
        else if (mMoveLeft)
        {
            moveDirection -= mCamera->mRightDirection;
        }
        moveDirection = glm::normalize(moveDirection) * 5.0f * deltaTime;
        mCamera->Translate(moveDirection);
    }

    if (mMouseDragCamera)
    {
        const float rotationAngle = 90.0f;
        if (mRotateDeltaY)
        {
            float angleRads = -glm::radians(glm::sign(mRotateDeltaY) * rotationAngle * deltaTime);
            glm::vec3 frontdir = glm::rotate(mCamera->mFrontDirection, angleRads, mCamera->mRightDirection);
            glm::vec3 updir = glm::normalize(glm::cross(mCamera->mRightDirection, frontdir)); 
            mCamera->SetOrientation(frontdir, mCamera->mRightDirection, updir);
        }

        if (mRotateDeltaX)
        {
            float angleRads = -glm::radians(glm::sign(mRotateDeltaX) * rotationAngle * deltaTime);
            glm::vec3 rightdir = glm::rotate(mCamera->mRightDirection, angleRads, SceneAxisY);
            glm::vec3 frontdir = glm::rotate(mCamera->mFrontDirection, angleRads, SceneAxisY);
            glm::vec3 updir = glm::normalize(glm::cross(rightdir, frontdir)); 
            mCamera->SetOrientation(frontdir, rightdir, updir);
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

    glm::vec3 position = mCamera->mPosition;
    position.y = glm::max(position.y + (MAP_BLOCK_LENGTH * 0.5f * -inputEvent.mScrollY), MAP_LAYERS_COUNT * MAP_BLOCK_LENGTH);
    mCamera->SetPosition(position);
}