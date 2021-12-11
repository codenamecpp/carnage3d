#pragma once

#include "AiDefs.h"

class AiPedestrianBehavior: public cxx::noncopyable
{
protected:

    //////////////////////////////////////////////////////////////////////////

    enum eAiActivityStatus
    {
        eAiActivityStatus_New,
        eAiActivityStatus_InProgress,
        eAiActivityStatus_Success,
        eAiActivityStatus_Failed,
        eAiActivityStatus_Cancelled
    };

    //////////////////////////////////////////////////////////////////////////
    // base activity
    class AiActivity: public cxx::noncopyable
    {
    public:
        // readonly
        AiPedestrianBehavior* mAiBehavior;
        eAiActivityStatus mActivityStatus = eAiActivityStatus_New;
    public:
        AiActivity(AiPedestrianBehavior* aiBehavior);
        virtual ~AiActivity();
        void StartActivity();
        void UpdateActivity();
        void CancelActivity();
        // status shortucts
        bool IsStatusInProgress() const;
        bool IsStatusSuccess() const;
        bool IsStatusFailed() const;
        bool IsStatusCancelled() const;
    protected:
        void SetActivityStatus(eAiActivityStatus newStatus);
        void StartChildActivity(AiActivity* childActivity);
        void ClearChildActivity();
        bool IsChildActivityInProgress() const;
        bool IsChildActivityInProgress(AiActivity* matchActivity) const;
        bool IsChildActivitySuccess() const;
        bool IsChildActivityFailed() const;
        // overridables
        virtual void OnActivityStart() {}
        virtual void OnActivityUpdate() {}
        virtual void OnActivityCancelled() {}
    protected:
        AiActivity* mParentActivity = nullptr;
        AiActivity* mChildActivity = nullptr;
    };

    //////////////////////////////////////////////////////////////////////////
    class AiActiviy_Wander: public AiActivity
    {
    public:
        AiActiviy_Wander(AiPedestrianBehavior* aiBehavior);
    protected:
        // override AiActivity
        void OnActivityUpdate() override;
    protected:
        bool ChooseDesiredPoint(eGroundType groundType);
    };

    //////////////////////////////////////////////////////////////////////////
    class AiActivity_Runaway: public AiActiviy_Wander
    {
    public:
        AiActivity_Runaway(AiPedestrianBehavior* aiBehavior);
    protected:
        // override AiActivity
        void OnActivityUpdate() override;
    protected:
        bool ChooseRunawayPoint();
    };

    //////////////////////////////////////////////////////////////////////////
    class AiActivity_FollowLeader: public AiActivity
    {
    public:
        AiActivity_FollowLeader(AiPedestrianBehavior* aiBehavior);
        // override AiActivity
        void OnActivityStart() override;
        void OnActivityUpdate() override;
    protected:
        bool CheckCanFollowTheLeader() const;
    };
    //////////////////////////////////////////////////////////////////////////

    class AiActivity_WalkToPoint: public AiActivity
    {
    public:
        AiActivity_WalkToPoint(AiPedestrianBehavior* aiBehavior);
        void SetRunning(bool setRunning);
        void SetArriveDistance(float arriveDistance);
        // override AiActivity
        void OnActivityStart() override;
        void OnActivityUpdate() override;
        void OnActivityCancelled() override;
    protected:
        bool ContinueWalk();
    protected:
        bool mSetRunning = false;
        float mArriveDistance = 0.0f;
    };

    //////////////////////////////////////////////////////////////////////////

    class AiActivity_Wait: public AiActivity
    {
    public:
        AiActivity_Wait(AiPedestrianBehavior* aiBehavior);
        void SetWaitSeconds(float waitSeconds);
        // override AiActivity
        void OnActivityStart() override;
        void OnActivityUpdate() override;
    protected:
        float mWaitSeconds = 0.0f;
        float mStartTime = 0.0f;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    // readonly
    eAiPedestrianBehavior mBehaviorID;

public:
    AiPedestrianBehavior(AiCharacterController* aiController, eAiPedestrianBehavior behaviorID);
    virtual ~AiPedestrianBehavior();

    void ActivateBehavior();
    void ShutdownBehavior();
    void UpdateBehavior();

    // memory bits
    void ChangeMemoryBits(AiBehaviorMemoryBits enableBits, AiBehaviorMemoryBits disableBits);
    bool CheckMemoryBits(AiBehaviorMemoryBits memoryBits) const;
    bool CheckMemoryBitsAll(AiBehaviorMemoryBits memoryBits) const;

    // behavior bits
    void ChangeBehaviorBits(AiBehaviorBits enableBits, AiBehaviorBits disableBits);
    bool CheckBehaviorBits(AiBehaviorBits bits) const;
    bool CheckBehaviorBitsAll(AiBehaviorBits bits) const;

    Pedestrian* GetCharacter() const;
    Pedestrian* GetLeader() const;
    
    void SetLeader(Pedestrian* pedestrian);

protected:
    // overridables
    virtual void OnActivateBehavior() {}
    virtual void OnShutdownBehavior() {}
    virtual void OnUpdateBehavior() {}
    virtual void ChooseDesiredActivity();

    void ScanForThreats();
    void ScanForLeader();

protected:
    AiCharacterController* mAiController = nullptr;
    AiActivity* mCurrentActivity = nullptr;
    AiActivity* mDesiredActivity = nullptr;

    AiBehaviorMemoryBits mMemoryBits = AiBehaviorMemoryBits_None;
    AiBehaviorBits mBehaviorBits = AiBehaviorBits_CanJump | AiBehaviorBits_Fear_GunShots | AiBehaviorBits_Fear_Explosions;

    // shared data
    glm::vec2 mDesiredPoint;
    PedestrianHandle mLeader;

    // standard activities
    AiActiviy_Wander mActivity_Wander;
    AiActivity_Runaway mActivity_Runaway;
    AiActivity_FollowLeader mActivity_FollowLeader;

    // primitive activities
    AiActivity_WalkToPoint mActivity_WalkToPoint;
    AiActivity_Wait mActivity_Wait;
};