#pragma once

class AiCharacterController;
class DebugRenderer;

// Artificial Intelligence manager class
class AiManager final: public cxx::noncopyable
{
public:
    AiManager();

    // Update all ai character controllers
    void UpdateFrame();
    void DebugDraw(DebugRenderer& debugRender);

    // Create ai controller for character
    // @param pedestrian: Character to control
    AiCharacterController* CreateAiController(Pedestrian* pedestrian);

    // Destroy ai character controller
    void ReleaseAiControllers();
    void ReleaseAiController(AiCharacterController* controller);

private:
    std::vector<AiCharacterController*> mCharacterControllers;
};

extern AiManager gAiManager;