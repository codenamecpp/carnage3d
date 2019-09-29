#include "stdafx.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "SpriteBatch.h"
#include "SpriteManager.h"
#include "RenderingManager.h"

Pedestrian::Pedestrian(unsigned int id)
    : GameObject(id)
    , mPhysicsComponent()
    , mDead()
    , mCurrentAnimID(eSpriteAnimationID_Null)
    , mControl(*this)
    , mActivePedsNode(this)
    , mDeletePedsNode(this)
{
}

Pedestrian::~Pedestrian()
{
    if (mPhysicsComponent)
    {
        gPhysics.DestroyPhysicsComponent(mPhysicsComponent);
    }
}

void Pedestrian::EnterTheGame()
{
    glm::vec3 startPosition;
    
    mPhysicsComponent = gPhysics.CreatePedPhysicsComponent(startPosition, 0.0f);
    debug_assert(mPhysicsComponent);

    mMarkForDeletion = false;
    mDead = false;
    mCurrentAnimID = eSpriteAnimationID_Null;
    // set initial state and animation
    SwitchToAnimation(eSpriteAnimationID_Ped_StandingStill, eSpriteAnimLoop_FromStart);
}

void Pedestrian::UpdateFrame(Timespan deltaTime)
{
    mAnimation.AdvanceAnimation(deltaTime);

    // ignore inputs when falling
    if (IsFalling())
    {
        SwitchToAnimation(eSpriteAnimationID_Ped_Falling, eSpriteAnimLoop_FromStart);
        return;
    }

    // try to turn around
    if (mControl.IsTurnAround())
    {
        mPhysicsComponent->SetAngularVelocity((mControl.mTurnLeft ? -1.0f : 1.0f) * gGameRules.mPedestrianTurnSpeed);
    }
    else
    {
        // stop rotation
        mPhysicsComponent->SetAngularVelocity(0.0f);
    }
    // try walk
    if (mControl.IsMoves())
    {
        float moveSpeed = 0.0f;
        bool moveBackward = false;
        if (mControl.mWalkForward)
        {
            if (mControl.mRunning)
            {
                moveSpeed = gGameRules.mPedestrianRunSpeed;
                SwitchToAnimation(eSpriteAnimationID_Ped_Run, eSpriteAnimLoop_FromStart);
            }
            else
            {
                moveSpeed = gGameRules.mPedestrianWalkSpeed;
                SwitchToAnimation(eSpriteAnimationID_Ped_Walk, eSpriteAnimLoop_FromStart);
            }
        }
        else if (mControl.mWalkBackward)
        {
            moveSpeed = gGameRules.mPedestrianWalkSpeed;
            moveBackward = true;
            SwitchToAnimation(eSpriteAnimationID_Ped_Walk, eSpriteAnimLoop_FromStart); // todo:reverse
        }
        // get current direction
        float angleRadians = mPhysicsComponent->GetAngleRadians();
        glm::vec2 signVector 
        {
            cos(angleRadians), sin(angleRadians)
        };

        if (moveBackward)
        {
            signVector = -signVector;
        }
        mPhysicsComponent->SetLinearVelocity(signVector * moveSpeed);
    }
    else
    {
        mPhysicsComponent->SetLinearVelocity({}); // force stop
        SwitchToAnimation(eSpriteAnimationID_Ped_StandingStill, eSpriteAnimLoop_FromStart);
    }
}

void Pedestrian::DrawFrame(SpriteBatch& spriteBatch)
{
    int spriteLinearIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Ped, mAnimation.GetCurrentFrame());
        
    float rotationAngle = glm::radians(mPhysicsComponent->GetAngleDegrees() - SPRITE_ZERO_ANGLE);

    glm::vec3 position = mPhysicsComponent->GetPosition();

    mDrawSprite.mTexture = gSpriteManager.mObjectsSpritesheet.mSpritesheetTexture;
    mDrawSprite.mTextureRegion = gSpriteManager.mObjectsSpritesheet.mEtries[spriteLinearIndex];
    mDrawSprite.mPosition = glm::vec2(position.x, position.z);
    mDrawSprite.mScale = SPRITE_SCALE;
    mDrawSprite.mRotateAngleRads = rotationAngle;
    mDrawSprite.mHeight = ComputeDrawHeight(position, rotationAngle);
    mDrawSprite.SetOriginToCenter();
    spriteBatch.DrawSprite(mDrawSprite);
}

void Pedestrian::SwitchToAnimation(eSpriteAnimationID animation, eSpriteAnimLoop loopMode)
{
    if (mCurrentAnimID != animation)
    {
        mAnimation.SetNull();
        if (!gGameMap.mStyleData.GetSpriteAnimation(animation, mAnimation.mAnimData)) // todo
        {
            debug_assert(false);
        }
        mCurrentAnimID = animation;
    }
    mAnimation.PlayAnimation(loopMode);
}

void Pedestrian::SetHeading(float angleDegrees)
{
    debug_assert(mPhysicsComponent);
    mPhysicsComponent->SetAngleDegrees(angleDegrees);
}

void Pedestrian::SetPosition(const glm::vec3& position)
{
    debug_assert(mPhysicsComponent);

    mPhysicsComponent->SetPosition(position);
}

bool Pedestrian::IsFalling() const
{
    debug_assert(mPhysicsComponent);

    return mPhysicsComponent->mFalling;
}

float Pedestrian::ComputeDrawHeight(const glm::vec3& position, float angleRadians)
{
    float halfBox = PED_SPRITE_DRAW_BOX_SIZE * 0.5f;

    //glm::vec3 points[4] = {
    //    { 0.0f,     position.y + 0.01f, -halfBox },
    //    { halfBox,  position.y + 0.01f, 0.0f },
    //    { 0.0f,     position.y + 0.01f, halfBox },
    //    { -halfBox, position.y + 0.01f, 0.0f },
    //};

    glm::vec3 points[4] = {
        { -halfBox, position.y + 0.01f, -halfBox },
        { halfBox,  position.y + 0.01f, -halfBox },
        { halfBox,  position.y + 0.01f, halfBox },
        { -halfBox, position.y + 0.01f, halfBox },
    };

    float maxHeight = position.y;
    for (glm::vec3& currPoint: points)
    {
        //currPoint = glm::rotate(currPoint, angleRadians, glm::vec3(0.0f, -1.0f, 0.0f)); // dont rotate for peds
        currPoint.x += position.x;
        currPoint.z += position.z;

        // get height
        float height = gGameMap.GetHeightAtPosition(currPoint);
        if (height > maxHeight)
        {
            maxHeight = height;
        }
    }
#if 1
    // debug
    for (int i = 0; i < 4; ++i)
    {
        gRenderManager.mDebugRenderer.DrawLine(points[i], points[(i + 1) % 4], COLOR_RED);
    }
#endif
    return maxHeight + 0.01f;
}
