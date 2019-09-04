#pragma once

enum eSpriteAnimStatus
{
    eSpriteAnimStatus_Stop,
    eSpriteAnimStatus_PlayForward,
    eSpriteAnimStatus_PlayBackward,
};

define_enum_strings(eSpriteAnimStatus)
{
    eSpriteAnimStatus_Stop, "stop",
    eSpriteAnimStatus_PlayForward, "play_forward",
    eSpriteAnimStatus_PlayBackward, "play_backward",
};

enum eSpriteAnimLoop
{
    eSpriteAnimLoop_None,
    eSpriteAnimLoop_PingPong,
    eSpriteAnimLoop_FromStart,
};

define_enum_strings(eSpriteAnimLoop)
{
    eSpriteAnimLoop_None, "none",
    eSpriteAnimLoop_PingPong, "ping_pong",
    eSpriteAnimLoop_FromStart, "from_start",
};

// defines sprite animation data
struct SpriteAnimationData
{
public:
    SpriteAnimationData() = default;
    inline void Setup(int startFrame, int numFrames, int fps = 12)
    {
        mFrameStart = startFrame;
        mFramesCount = numFrames;
        mFramesPerSecond = fps;
    }
    inline void SetNull()
    {
        mFrameStart = 0;
        mFramesCount = 0;
        mFramesPerSecond = 0;
    }
    inline bool IsNull() const { return mFramesCount == 0; }
    inline bool NonNull() const { return mFramesCount > 0; }

public:
    int mFrameStart = 0;
    int mFramesCount = 0;
    int mFramesPerSecond = 0;
};

// defines sprite animation instance
class SpriteAnimation final
{
public:
    SpriteAnimation();

    // advance animation state
    // @param deltaTime: Time since last frame
    void UpdateFrame(Timespan deltaTime);

    // clear animation
    void SetNull();

    // animation control
    void StopAnimation();
    void PlayAnimation(eSpriteAnimLoop animLoop);
    void PlayAnimation(eSpriteAnimLoop animLoop, int fps);
    void RewindToStart();
    void RewindToEnd();

    void NextFrame(bool moveForward);

    // test whether animation in progress
    bool IsAnimationActive() const;
    
public:
    // public for convenience, should not be modified directly
    SpriteAnimationData mAnimData;
    eSpriteAnimStatus mStatus;
    eSpriteAnimLoop mLoopMode;

    Timespan mTicksFromFrameStart;
    Timespan mTicksFromAnimStart;
    
    int mCyclesCounter;
    // frame index is relative to sprite type start offset
    int mCurrentFrame;
};