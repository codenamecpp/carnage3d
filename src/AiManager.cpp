#include "stdafx.h"
#include "AiManager.h"
#include "AiCharacterController.h"
#include "Pedestrian.h"

AiManager gAiManager;

AiManager::AiManager()
{
}

void AiManager::UpdateFrame()
{
    // update all character controllers
    bool hasInactiveControllers = false;
    for (size_t iController = 0, Count = mCharacterControllers.size(); iController < Count; ++iController)
    {
        AiCharacterController* currController = mCharacterControllers[iController];
        if (currController->IsActive())
        {
            currController->UpdateFrame();
        }
        else
        {
            SafeDelete(mCharacterControllers[iController]);
            hasInactiveControllers = true;
        }
    }
    if (hasInactiveControllers)
    {
        cxx::erase_elements(mCharacterControllers, nullptr);
    }
}

void AiManager::DebugDraw(DebugRenderer& debugRender)
{
}

void AiManager::ReleaseAiControllers()
{
    for (AiCharacterController* currController: mCharacterControllers)
    {
        delete currController;
    }

    mCharacterControllers.clear();
}

AiCharacterController* AiManager::CreateAiController(Pedestrian* pedestrian)
{
    if (pedestrian == nullptr)
    {
        debug_assert(false);
        return nullptr;
    }

    if (pedestrian->mController)
    {
        debug_assert(false);
        return nullptr;
    }

    AiCharacterController* controller = new AiCharacterController(pedestrian);
    mCharacterControllers.push_back(controller);
    return controller;
}

void AiManager::ReleaseAiController(AiCharacterController* controller)
{
    if (controller == nullptr)
    {
        debug_assert(false);
        return;
    }

    cxx::erase_elements(mCharacterControllers, controller);
    delete controller;
}
