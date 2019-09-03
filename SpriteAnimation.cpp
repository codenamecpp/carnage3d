#include "stdafx.h"
#include "SpriteAnimation.h"

SpriteAnimation::SpriteAnimation(): mCurrentFrame()
{
    SetNull();
}

void SpriteAnimation::UpdateFrame(Timespan deltaTime)
{
    if (mStatus == eSpriteAnimStatus_Stop)
        return;
}

void SpriteAnimation::SetNull()
{
    mAnimData.SetNull();
    mStatus = eSpriteAnimStatus_Stop;
    mLoopMode = eSpriteAnimLoop_None;

    mFrameStartTimestamp.SetNull();
    mAnimStartTimestamp.SetNull();
}

bool SpriteAnimation::IsAnimationActive() const
{
    return mStatus != eSpriteAnimStatus_Stop;
}

void SpriteAnimation::PlayAnimation(eSpriteAnimLoop animLoop)
{
    if (mStatus != eSpriteAnimStatus_Stop)
        return;

}

void SpriteAnimation::StopAnimation()
{
    if (mStatus == eSpriteAnimStatus_Stop)
        return;

}

void SpriteAnimation::RewindToStart()
{
}

void SpriteAnimation::RewindToEnd()
{
}
