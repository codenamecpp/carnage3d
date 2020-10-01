#include "stdafx.h"
#include "SpriteAnimation.h"

bool SpriteAnimData::Deserialize(cxx::json_document_node configNode)
{
    Clear();

    cxx::json_get_attribute(configNode, "fps", mFrameRate);

    int numFrames = 0;
    if (cxx::json_node_array framesNode = configNode["frames"])
    {
        numFrames = framesNode.get_elements_count();
        mFrames.resize(numFrames);
        // get frames
        for (int icurrFrame = 0; icurrFrame < numFrames; ++icurrFrame)
        {
            if (!cxx::json_get_attribute(framesNode, icurrFrame, mFrames[icurrFrame].mSprite))
            {
                debug_assert(false);
            }
        }
    }
    else
    {
        int baseFrame = 0;
        if (!cxx::json_get_attribute(configNode, "start_frame", baseFrame) ||
            !cxx::json_get_attribute(configNode, "num_frames", numFrames))
        {
            debug_assert(false);
        }
        SetFrames(baseFrame, numFrames);
    }

    // read frame actions
    if (cxx::json_node_array actionsNode = configNode["actions"])
    {
        for (cxx::json_node_object currActionNode = actionsNode.first_child();
            currActionNode; currActionNode = currActionNode.next_sibling())
        {
            int frameIndex = 0;
            if (!cxx::json_get_attribute(currActionNode, "frame_index", frameIndex))
                continue;

            if (frameIndex >= numFrames)
            {
                debug_assert(false);
                continue;
            }

            cxx::json_get_attribute(currActionNode, "action", mFrames[frameIndex].mActionID);
        }
    }

    return true;
}

void SpriteAnimData::SetFrames(int startSpriteIndex, int numFrames)
{
    mFrames.resize(numFrames);
    for (int iframe = 0; iframe < numFrames; ++iframe)
    {
        mFrames[iframe].mActionID = eSpriteAnimAction_None;
        mFrames[iframe].mSprite = startSpriteIndex + iframe;
    }
}

void SpriteAnimData::SetFrames(std::initializer_list<int> frames)
{
    mFrames.resize(frames.size());

    for (int iframe = 0, Count = (int) frames.size(); iframe < Count; ++iframe)
    {
        mFrames[iframe].mActionID = eSpriteAnimAction_None;
        mFrames[iframe].mSprite = frames.begin()[iframe];
    }
}

void SpriteAnimData::Clear()
{
    mFrameRate = SPRITE_ANIM_DEFAULT_FPS;
    mFrames.clear();
}

void SpriteAnimData::ClearFrameActions()
{
    for (SpriteAnimFrame& currFrame: mFrames)
    {
        currFrame.mActionID = eSpriteAnimAction_None;
    }
}

int SpriteAnimData::GetFramesCount() const
{
    return (int) mFrames.size();
}

//////////////////////////////////////////////////////////////////////////

SpriteAnimation::SpriteAnimation(): mFrameCursor(), mCyclesCounter()
{
    Clear();
}

void SpriteAnimation::SetListener(SpriteAnimListener* animationListener)
{
    mListener = animationListener;
}

void SpriteAnimation::Clear()
{
    ClearState();

    mAnimDesc.Clear();
}

void SpriteAnimation::ClearState()
{
    mState = eSpriteAnimState_Stopped;
    mLoopMode = eSpriteAnimLoop_None;
    mPlayMode = eSpriteAnimMode_Normal;
    mFrameCursor = 0;
    mLastFrameCursor = -1;
    mCyclesCounter = 0;
    mFrameTime = 0.0f;
    mFireActionsQueue.clear();
}

bool SpriteAnimation::IsActive() const
{
    return mState != eSpriteAnimState_Stopped;
}

bool SpriteAnimation::IsPaused() const
{
    return mState == eSpriteAnimState_Paused;
}

bool SpriteAnimation::IsStopped() const
{
    return mState == eSpriteAnimState_Stopped;
}

bool SpriteAnimation::IsFirstFrame() const
{
    return mFrameCursor == 0;
}

bool SpriteAnimation::IsLastFrame() const
{
    int framesCount = mAnimDesc.GetFramesCount();
    return (framesCount > 0) && mFrameCursor == (framesCount - 1);
}

bool SpriteAnimation::IsNull() const
{
    return mAnimDesc.GetFramesCount() == 0;
}

bool SpriteAnimation::IsRunsForwards() const
{
    return (mState == eSpriteAnimState_Play) && (mPlayMode == eSpriteAnimMode_Normal);
}

bool SpriteAnimation::IsRunsInReverse() const
{
    return (mState == eSpriteAnimState_Play) && (mPlayMode == eSpriteAnimMode_Reverse);
}

void SpriteAnimation::PlayAnimation(eSpriteAnimLoop animLoop, eSpriteAnimMode playMode)
{
    int currentFrameCursor = mFrameCursor;
    ClearState();
    mFrameCursor = currentFrameCursor;

    if (IsNull() || mAnimDesc.mFrameRate < 0.001f)
    {
        debug_assert(false);
        return;
    }
    mLoopMode = animLoop;
    mPlayMode = playMode;
    mState = eSpriteAnimState_Play;
}

void SpriteAnimation::PlayAnimation(eSpriteAnimLoop animLoop, eSpriteAnimMode playMode, float fps)
{
    mAnimDesc.mFrameRate = fps;
    PlayAnimation(animLoop, playMode);
}

void SpriteAnimation::SetMaxRepeatCycles(int numCycles)
{
    mCyclesCountMax = numCycles;
    if (mCyclesCountMax < 0)
    {
        debug_assert(false);
        mCyclesCountMax = 0; // infinite
    }
}

void SpriteAnimation::SetCurrentLoop(eSpriteAnimLoop animLoop)
{
    mLoopMode = animLoop;
}

void SpriteAnimation::SetCurrentMode(eSpriteAnimMode animMode)
{
    mPlayMode = animMode;
}

void SpriteAnimation::StopAnimation()
{
    mState = eSpriteAnimState_Stopped;
}

void SpriteAnimation::ContinueAnimation()
{
    if (mState == eSpriteAnimState_Paused)
    {
        mState = eSpriteAnimState_Play;
    }
}

void SpriteAnimation::PauseAnimation()
{
    if (mState == eSpriteAnimState_Play)
    {
        mState = eSpriteAnimState_Paused;
    }
}

void SpriteAnimation::RewindToStart()
{
    mFrameCursor = 0;
}

void SpriteAnimation::RewindToEnd()
{
    mFrameCursor = 0;
    int framesCount = mAnimDesc.GetFramesCount();
    if (framesCount > 0)
    {
        mFrameCursor = framesCount - 1;
    }
}

bool SpriteAnimation::UpdateFrame(float deltaTime)
{
    if ((mState == eSpriteAnimState_Stopped) || (mState == eSpriteAnimState_Paused))
    {
        // false means nothing changed
        return false;
    }

    if (mLastFrameCursor != mFrameCursor)
    {
        QueueFrameAction();
    }

    mFrameTime += deltaTime;

    const float FrameDuration = (1.0f / mAnimDesc.mFrameRate);

    int advanceFramesCount = 0;
    for (; mFrameTime >= FrameDuration; ++advanceFramesCount)
    {
        mFrameTime -= FrameDuration;

        AdvanceProgressForSingleFrame();
        QueueFrameAction();

        if (mState == eSpriteAnimState_Stopped)
        {
            break;
        }
    }

    FireFrameActions();
    return true;
}

void SpriteAnimation::NextFrame(bool moveForward)
{
    if (moveForward)
    {
        if (mFrameCursor < (mAnimDesc.GetFramesCount() - 1))
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

void SpriteAnimation::AdvanceProgressForSingleFrame()
{
    if (mPlayMode == eSpriteAnimMode_Normal)
    {
        if (mFrameCursor == (mAnimDesc.GetFramesCount() - 1)) // end
        {
            ++mCyclesCounter;
            if (mCyclesCountMax > 0 && (mCyclesCounter >= mCyclesCountMax))
            {
                mLoopMode = eSpriteAnimLoop_None;
            }

            switch (mLoopMode)
            {
                case eSpriteAnimLoop_None:
                    mState = eSpriteAnimState_Stopped;
                break;

                case eSpriteAnimLoop_FromStart:
                    mFrameCursor = 0;
                break;

                case eSpriteAnimLoop_PingPong:
                    mPlayMode = eSpriteAnimMode_Reverse;
                    NextFrame(false);
                break;
            }
        }
        else
        {
            NextFrame(true);
        }
    }
    else if (mPlayMode == eSpriteAnimMode_Reverse)
    {
        if (mFrameCursor == 0) // end
        {
            ++mCyclesCounter;
            if (mCyclesCountMax > 0 && (mCyclesCounter >= mCyclesCountMax))
            {
                mLoopMode = eSpriteAnimLoop_None;
            }

            switch (mLoopMode)
            {
                case eSpriteAnimLoop_None:
                    mState = eSpriteAnimState_Stopped;
                break;

                case eSpriteAnimLoop_FromStart:
                break;

                case eSpriteAnimLoop_PingPong:
                    mPlayMode = eSpriteAnimMode_Normal;
                    NextFrame(true);
                break;
            }
        }
        else
        {
            NextFrame(false);
        }
    }
}

void SpriteAnimation::QueueFrameAction()
{
    if (mLastFrameCursor != mFrameCursor)
    {
        if (mListener)
        {
            SpriteAnimFrame& frame = mAnimDesc.mFrames[mFrameCursor];
            if (frame.mActionID != eSpriteAnimAction_None)
            {
                mFireActionsQueue.push_back(mFrameCursor);
            }
        }
        mLastFrameCursor = mFrameCursor;
    }
}

void SpriteAnimation::FireFrameActions()
{
    if (mFireActionsQueue.empty())
        return;

    if (mListener == nullptr)
    {
        debug_assert(mFireActionsQueue.empty());
        mFireActionsQueue.clear();
        return;
    }

    while (!mFireActionsQueue.empty())
    {
        int frameCursor = mFireActionsQueue.front();
        mFireActionsQueue.pop_front();

        SpriteAnimFrame& frame = mAnimDesc.mFrames[frameCursor];  
        if (mListener == nullptr || !mListener->OnAnimFrameAction(this, frameCursor, frame.mActionID))
        {
            mFireActionsQueue.clear();
            break;
        }
    }
}

int SpriteAnimation::GetSpriteIndex() const
{
    int framesCount = mAnimDesc.GetFramesCount();
    if (framesCount > 0)
    {
        debug_assert(framesCount > mFrameCursor);

        const SpriteAnimFrame& animFrame = mAnimDesc.mFrames[mFrameCursor];
        return animFrame.mSprite;
    }
    return 0;
}