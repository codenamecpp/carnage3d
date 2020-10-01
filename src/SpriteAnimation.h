#pragma once

#define SPRITE_ANIM_DEFAULT_FPS 12.0f

// forwards
class SpriteAnimation;

// Sprite animation playback mode
enum eSpriteAnimMode
{
    eSpriteAnimMode_Normal,
    eSpriteAnimMode_Reverse
};

decl_enum_strings(eSpriteAnimMode);

// Sprite animation playback state
enum eSpriteAnimState
{
    eSpriteAnimState_Play,
    eSpriteAnimState_Stopped,
    eSpriteAnimState_Paused,
};

decl_enum_strings(eSpriteAnimState);

// Sprite animation loop mode
enum eSpriteAnimLoop
{
    eSpriteAnimLoop_None,
    eSpriteAnimLoop_PingPong,
    eSpriteAnimLoop_FromStart,
};

decl_enum_strings(eSpriteAnimLoop);

// Sprite animation frame action type
enum eSpriteAnimAction
{
    eSpriteAnimAction_None,
    eSpriteAnimAction_CarDoorOpen,
    eSpriteAnimAction_CarDoorClose,
    eSpriteAnimAction_Footstep,
};

decl_enum_strings(eSpriteAnimAction);

// defines sprite animation listener class
class SpriteAnimListener
{
public:
    virtual ~SpriteAnimListener()
    {
    }
    // Handle frame action, return false to drop all queued actions since last update
    // @param animation: Animation instance
    // @param frameIndex: Frame index with action
    // @param actionID: Sprite action identifier
    virtual bool OnAnimFrameAction(SpriteAnimation* animation, int frameIndex, eSpriteAnimAction actionID)
    {
        return true;
    }
};

// defines sprite animation frame data
struct SpriteAnimFrame
{
public:
    int mSprite = 0; // index of sprite texture in spritesheet

    // optional frame action identifier
    eSpriteAnimAction mActionID = eSpriteAnimAction_None; 
};

// defines sprite animation data
struct SpriteAnimData
{
public:
    SpriteAnimData() = default;

    // Read animation data from json node, returns false on error
    bool Deserialize(cxx::json_document_node configNode);
    void Clear();
    void ClearFrameActions();

    // Setup animation frames
    void SetFrames(int startSpriteIndex, int framesCount);

    // Setup animation frames with specific sprite indices
    void SetFrames(std::initializer_list<int> frames);

    int GetFramesCount() const;
public:
    std::vector<SpriteAnimFrame> mFrames;
    float mFrameRate = SPRITE_ANIM_DEFAULT_FPS; // frames per second
};

// defines sprite animation state
class SpriteAnimation
{
public:
    // readonly
    SpriteAnimData mAnimDesc;

    eSpriteAnimMode mPlayMode = eSpriteAnimMode_Normal;
    eSpriteAnimState mState;
    eSpriteAnimLoop mLoopMode;

    float mFrameTime; // time accumulator
    int mCyclesCounter;
    int mCyclesCountMax = 0;
    int mFrameCursor; // current offset in mAnimData.Frames
    int mLastFrameCursor = -1;

public:
    SpriteAnimation();
    void SetListener(SpriteAnimListener* animationListener);

    // advance animation state, returns true on frame changes
    bool UpdateFrame(float deltaTime);
    void Clear();
    void ClearState();

    // start animation from _current_ position, resets previous playback state
    void PlayAnimation(eSpriteAnimLoop animLoop, eSpriteAnimMode playMode = eSpriteAnimMode_Normal);
    void PlayAnimation(eSpriteAnimLoop animLoop, eSpriteAnimMode playMode, float fps);
    void StopAnimation();

    // hold and restore animation without reseting playback state
    void PauseAnimation();
    void ContinueAnimation();

    void SetMaxRepeatCycles(int numCycles);

    void SetCurrentLoop(eSpriteAnimLoop animLoop);
    void SetCurrentMode(eSpriteAnimMode animMode);

    void RewindToStart();
    void RewindToEnd();

    int GetSpriteIndex() const;
    
    bool IsActive() const; // test whether animation in progress or paused
    bool IsPaused() const;
    bool IsStopped() const;

    bool IsFirstFrame() const;
    bool IsLastFrame() const;
    bool IsRunsForwards() const;
    bool IsRunsInReverse() const;

    bool IsNull() const;

private:
    void NextFrame(bool moveForward);
    void AdvanceProgressForSingleFrame();
    void QueueFrameAction();
    void FireFrameActions();
    
private:
    SpriteAnimListener* mListener = nullptr;
    std::deque<int> mFireActionsQueue;
};