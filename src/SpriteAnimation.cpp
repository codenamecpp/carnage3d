#include "stdafx.h"
#include "SpriteAnimation.h"

SpriteAnimation::SpriteAnimation(): mFrameCursor(), mCyclesCounter()
{
    SetNull();
}

void SpriteAnimation::SetNull()
{
    mStatus = eSpriteAnimStatus_Stop;
    mLoopMode = eSpriteAnimLoop_None;
    mFrameCursor = 0;
    mCyclesCounter = 0;

    mAnimDesc.SetNull();
}

bool SpriteAnimation::IsAnimationActive() const
{
    return mStatus != eSpriteAnimStatus_Stop;
}

bool SpriteAnimation::IsFirstFrame() const
{
    return mFrameCursor == 0;
}

bool SpriteAnimation::IsLastFrame() const
{
    return (mAnimDesc.mFramesCount > 0) && mFrameCursor == (mAnimDesc.mFramesCount - 1);
}

bool SpriteAnimation::IsNull() const
{
    return mAnimDesc.IsNull();
}

bool SpriteAnimation::IsRunsForwards() const
{
    return mStatus == eSpriteAnimStatus_PlayForward;
}

bool SpriteAnimation::IsRunsBackwards() const
{
    return mStatus == eSpriteAnimStatus_PlayBackward;
}

void SpriteAnimation::PlayAnimation(eSpriteAnimLoop animLoop)
{
    if (mStatus != eSpriteAnimStatus_Stop)
        return;

    if (mAnimDesc.mFramesCount < 1 || mAnimDesc.mFramesPerSecond < 0.001f)
    {
        debug_assert(false);
        return;
    }

    mFrameStartTime = 0.0;
    mAnimationStartTime = 0.0;
    mCyclesCounter = 0;
    mStatus = eSpriteAnimStatus_PlayForward;
    mLoopMode = animLoop;
}

void SpriteAnimation::PlayAnimation(eSpriteAnimLoop animLoop, float fps)
{
    mAnimDesc.mFramesPerSecond = fps;

    PlayAnimation(animLoop);
}

void SpriteAnimation::PlayAnimationBackwards(eSpriteAnimLoop animLoop)
{
    PlayAnimation(animLoop);

    mStatus = eSpriteAnimStatus_PlayBackward;
}

void SpriteAnimation::PlayAnimationBackwards(eSpriteAnimLoop animLoop, float fps)
{
    PlayAnimation(animLoop, fps);

    mStatus = eSpriteAnimStatus_PlayBackward;
}

void SpriteAnimation::StopAnimation()
{
    mStatus = eSpriteAnimStatus_Stop;
}

void SpriteAnimation::StopAnimationAtEnd()
{
    mLoopMode = eSpriteAnimLoop_None;
}

void SpriteAnimation::RewindToStart()
{
    mFrameCursor = 0;
}

void SpriteAnimation::RewindToEnd()
{
    mFrameCursor = 0;
    if (mAnimDesc.mFramesCount > 0)
    {
        mFrameCursor = (mAnimDesc.mFramesCount - 1);
    }
}

bool SpriteAnimation::AdvanceAnimation(float deltaTime)
{
    if (mStatus == eSpriteAnimStatus_Stop)
        return false;

    mAnimationStartTime += deltaTime;
    mFrameStartTime += deltaTime;

    float ticksPerFrame = (1.0f / mAnimDesc.mFramesPerSecond);
    if (mFrameStartTime < ticksPerFrame)
        return false;

    // start next frame
    mFrameStartTime = 0.0f;
    if (mStatus == eSpriteAnimStatus_PlayForward)
    {
        if (mFrameCursor == (mAnimDesc.mFramesCount - 1)) // end
        {
            switch (mLoopMode)
            {
                case eSpriteAnimLoop_None:
                    mStatus = eSpriteAnimStatus_Stop;
                break;

                case eSpriteAnimLoop_FromStart:
                    mFrameCursor = 0;
                break;

                case eSpriteAnimLoop_PingPong:
                    mStatus = eSpriteAnimStatus_PlayBackward;
                    NextFrame(false);
                break;
            }
            ++mCyclesCounter;
        }
        else
        {
            NextFrame(true);
        }
    }
    else if (mStatus == eSpriteAnimStatus_PlayBackward)
    {
        if (mFrameCursor == 0) // end
        {
            switch (mLoopMode)
            {
                case eSpriteAnimLoop_None:
                    mStatus = eSpriteAnimStatus_Stop;
                break;

                case eSpriteAnimLoop_FromStart:
                break;

                case eSpriteAnimLoop_PingPong:
                    mStatus = eSpriteAnimStatus_PlayForward;
                    NextFrame(true);
                break;
            } 
            ++mCyclesCounter;
        }
        else
        {
            NextFrame(false);
        }
    }        
    return true;
}

void SpriteAnimation::NextFrame(bool moveForward)
{
    if (moveForward)
    {
        if (mFrameCursor < (mAnimDesc.mFramesCount - 1))
        {
            ++mFrameCursor;
        }
    }
    else
    {
        if (mFrameCursor > 0)
        {
            --mFrameCursor;
        }
    }
}

int SpriteAnimation::GetCurrentFrame() const
{
    if (mAnimDesc.mFramesCount > 0)
    {
        debug_assert(mAnimDesc.mFramesCount > mFrameCursor);
        return mAnimDesc.mFrames[mFrameCursor];
    }
    return 0;
}
