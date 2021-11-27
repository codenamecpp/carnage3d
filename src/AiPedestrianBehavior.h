#pragma once

class AiCharacterController;

class AiPedestrianBehavior: public cxx::noncopyable
{
public:
    AiPedestrianBehavior(AiCharacterController* aiController);
    virtual ~AiPedestrianBehavior();

    void ActivateBehavior();
    void ShutdownBehavior();
    void UpdateBehavior();

protected:
    // overridables
    virtual void OnActivateBehavior();
    virtual void OnShutdownBehavior();

protected:

protected:
    AiCharacterController* mAiController = nullptr;
};