#include "stdafx.h"
#include "SpriteAnimation.h"

SpriteAnimation::SpriteAnimation(): mCurrentFrame(), mCyclesCounter()
{
    SetNull();
}

void SpriteAnimation::SetNull()
{
    mStatus = eSpriteAnimStatus_Stop;
    mLoopMode = eSpriteAnimLoop_None;

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

    if (mAnimData.mFramesCount < 1 || mAnimData.mFramesPerSecond < 1)
    {
        debug_assert(false);
        return;
    }

    mTicksFromFrameStart = 0;
    mTicksFromAnimStart = 0;
    mCyclesCounter = 0;
    mCurrentFrame = mAnimData.mFrameStart;
    mStatus = eSpriteAnimStatus_PlayForward;
    mLoopMode = animLoop;
}

void SpriteAnimation::PlayAnimation(eSpriteAnimLoop animLoop, int fps)
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
    mCurrentFrame = mAnimData.mFrameStart;
}

void SpriteAnimation::RewindToEnd()
{
    mCurrentFrame = mAnimData.mFrameStart;
    if (mAnimData.mFramesCount > 0)
    {
        mCurrentFrame += (mAnimData.mFramesCount - 1);
    }
}

void SpriteAnimation::UpdateFrame(Timespan deltaTime)
{
    if (mStatus == eSpriteAnimStatus_Stop)
        return;

    mTicksFromAnimStart += deltaTime;
    mTicksFromFrameStart += deltaTime;

    Timespan ticksPerFrame = Timespan::FromSeconds(1.0f / mAnimData.mFramesPerSecond);
    if (mTicksFromFrameStart < ticksPerFrame)
        return;

    // start next frame
    mTicksFromFrameStart = 0;
    if (mStatus == eSpriteAnimStatus_PlayForward)
    {
        if (mCurrentFrame == (mAnimData.mFrameStart + mAnimData.mFramesCount - 1)) // end
        {
            switch (mLoopMode)
            {
                case eSpriteAnimLoop_None:
                    mStatus = eSpriteAnimStatus_Stop;
                break;

                case eSpriteAnimLoop_FromStart:
                    mCurrentFrame = mAnimData.mFrameStart;
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

        return;
    }

    if (mStatus == eSpriteAnimStatus_PlayBackward)
    {
        if (mCurrentFrame == mAnimData.mFrameStart) // end
        {
            switch (mLoopMode)
            {
                case eSpriteAnimLoop_None:
                    mStatus = eSpriteAnimStatus_Stop;
                break;

                case eSpriteAnimLoop_FromStart:
                    mCurrentFrame = mAnimData.mFrameStart;
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

        return;
    }        
}

void SpriteAnimation::NextFrame(bool moveForward)
{
    if (moveForward)
    {
        if (mCurrentFrame < mAnimData.mFrameStart + mAnimData.mFramesCount - 1)
        {
            ++mCurrentFrame;
        }
    }
    else
    {
        if (mCurrentFrame > mAnimData.mFrameStart)
        {
            --mCurrentFrame;
        }
    }
}
