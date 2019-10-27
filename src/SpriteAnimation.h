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
        debug_assert(mFramesCount <= MaxSpriteAnimationFrames);
        for (int iframe = 0; iframe < mFramesCount; ++iframe)
        {
            mFrames[iframe] = startFrame + iframe;
        }
        mFramesPerSecond = fps;
    }
    inline void SetupFrames(std::initializer_list<int> frames, float fps = 12.0f)
    {
        mFramesCount = frames.size();
        debug_assert(mFramesCount <= MaxSpriteAnimationFrames);
        for (int iframe = 0; iframe < mFramesCount; ++iframe)
        {
            mFrames[iframe] = *(frames.begin() + iframe);
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
    void StopAnimationAtEnd(); // disable current loop mode

    // play animation from current position
    void PlayAnimation(eSpriteAnimLoop animLoop);
    void PlayAnimation(eSpriteAnimLoop animLoop, float fps);
    void PlayAnimationBackwards(eSpriteAnimLoop animLoop);
    void PlayAnimationBackwards(eSpriteAnimLoop animLoop, float fps);

    void RewindToStart();
    void RewindToEnd();
    void NextFrame(bool moveForward);
    int GetCurrentFrame() const;
    
    bool IsAnimationActive() const; // test whether animation in progress
    bool IsFirstFrame() const;
    bool IsLastFrame() const;
    bool IsRunsForwards() const;
    bool IsRunsBackwards() const;

    bool IsNull() const;
    
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