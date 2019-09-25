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

    mAnimData.SetNull();
}

bool SpriteAnimation::IsAnimationActive() const
{
    return mStatus != eSpriteAnimStatus_Stop;
}

void SpriteAnimation::PlayAnimation(eSpriteAnimLoop animLoop)
{
    if (mStatus != eSpriteAnimStatus_Stop)
        return;

    if (mAnimData.mFramesCount < 1 || mAnimData.mFramesPerSecond < 0.001f)
    {
        debug_assert(false);
        return;
    }

    mTicksFromFrameStart = 0;
    mTicksFromAnimStart = 0;
    mCyclesCounter = 0;
    mFrameCursor = 0;
    mStatus = eSpriteAnimStatus_PlayForward;
    mLoopMode = animLoop;
}

void SpriteAnimation::PlayAnimation(eSpriteAnimLoop animLoop, float fps)
{
    mAnimData.mFramesPerSecond = fps;

    PlayAnimation(animLoop);
}

void SpriteAnimation::StopAnimation()
{
    mStatus = eSpriteAnimStatus_Stop;
}

void SpriteAnimation::RewindToStart()
{
    mFrameCursor = 0;
}

void SpriteAnimation::RewindToEnd()
{
    mFrameCursor = 0;
    if (mAnimData.mFramesCount > 0)
    {
        mFrameCursor = (mAnimData.mFramesCount - 1);
    }
}

bool SpriteAnimation::AdvanceAnimation(Timespan deltaTime)
{
    if (mStatus == eSpriteAnimStatus_Stop)
        return false;

    mTicksFromAnimStart += deltaTime;
    mTicksFromFrameStart += deltaTime;

    float ticksPerFrame = (1.0f / mAnimData.mFramesPerSecond);
    if (mTicksFromFrameStart.ToSeconds() < ticksPerFrame)
        return false;

    // start next frame
    mTicksFromFrameStart = 0;
    if (mStatus == eSpriteAnimStatus_PlayForward)
    {
        if (mFrameCursor == (mAnimData.mFramesCount - 1)) // end
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
        if (mFrameCursor < (mAnimData.mFramesCount - 1))
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
    if (mAnimData.mFramesCount > 0)
    {
        debug_assert(mAnimData.mFramesCount > mFrameCursor);
        return mAnimData.mFrames[mFrameCursor];
    }
    return 0;
}
