#pragma once

#define SPRITES_ANIM_DEFAULT_FPS 12.0f

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
struct SpriteAnimData
{
public:
    SpriteAnimData() = default;
    inline void Setup(int startFrame, int numFrames, float fps = SPRITES_ANIM_DEFAULT_FPS)
    {
        mFramesCount = numFrames;
        debug_assert(mFramesCount <= MaxSpriteAnimationFrames);
        for (int iframe = 0; iframe < mFramesCount; ++iframe)
        {
            mFrames[iframe] = startFrame + iframe;
        }
        mFramesPerSecond = fps;
    }
    inline void SetupFrames(std::initializer_list<int> frames, float fps = SPRITES_ANIM_DEFAULT_FPS)
    {
        mFramesCount = frames.size();
        debug_assert(mFramesCount <= MaxSpriteAnimationFrames);
        for (int iframe = 0; iframe < mFramesCount; ++iframe)
        {
            mFrames[iframe] = *(frames.begin() + iframe);
        }
        mFramesPerSecond = fps;
    }
    inline void Clear()
    {
        mFramesCount = 0;
        mFramesPerSecond = 0.0f;
    }
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
    bool AdvanceAnimation(float deltaTime);
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
    SpriteAnimData mAnimDesc;
    eSpriteAnimStatus mStatus;
    eSpriteAnimLoop mLoopMode;

    float mFrameStartTime;
    float mAnimationStartTime;
    
    int mCyclesCounter;
    int mFrameCursor; // current offset in mAnimData.Frames
};