#pragma once

const int MaxSpriteAnimationFrames = 64;

enum eSpriteAnimStatus
{
    eSpriteAnimStatus_Stop,
    eSpriteAnimStatus_PlayForward,
    eSpriteAnimStatus_PlayBackward,
};

decl_enum_strings(eSpriteAnimStatus);

enum eSpriteAnimLoop
{
    eSpriteAnimLoop_None,
    eSpriteAnimLoop_PingPong,
    eSpriteAnimLoop_FromStart,
};

decl_enum_strings(eSpriteAnimLoop);

// defines sprite animation data
struct SpriteAnimationData
{
public:
    SpriteAnimationData() = default;
    inline void Setup(int startFrame, int numFrames, float fps = 12.0f)
    {
        mFramesCount = numFrames;
        for (int iframe = 0; iframe < numFrames; ++iframe)
        {
            mFrames[iframe] = startFrame + iframe;
        }
        mFramesPerSecond = fps;
    }
    inline void SetNull()
    {
        mFramesCount = 0;
        mFramesPerSecond = 0.0f;
    }
    inline bool IsNull() const { return mFramesCount == 0; }
    inline bool NonNull() const { return mFramesCount > 0; }

public:
    int mFrames[MaxSpriteAnimationFrames];
    int mFramesCount = 0;
    float mFramesPerSecond = 0.0f;
};

// defines sprite animation state
class SpriteAnimation
{
public:
    SpriteAnimation();
    // advance animation state, returns true on frame changes
    bool AdvanceAnimation(Timespan deltaTime);
    void SetNull();

    // animation control
    void StopAnimation();
    void PlayAnimation(eSpriteAnimLoop animLoop);
    void PlayAnimation(eSpriteAnimLoop animLoop, float fps);
    void RewindToStart();
    void RewindToEnd();
    void NextFrame(bool moveForward);
    int GetCurrentFrame() const;
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
    int mFrameCursor; // current offset in mAnimData.Frames
};