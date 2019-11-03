#include "stdafx.h"
#include "GameCheatsWindow.h"
#include "imgui.h"
#include "RenderingManager.h"
#include "PhysicsManager.h"
#include "CarnageGame.h"
#include "Pedestrian.h"

GameCheatsWindow gGameCheatsWindow;

GameCheatsWindow::GameCheatsWindow()
    : DebugWindow("Game Cheats")
    , mEnableMapCollisions(true)
    , mEnableGravity(true)
    , mEnableBlocksAnimation(true)
{
    for (int ilayer = 0; ilayer < MAP_LAYERS_COUNT; ++ilayer)
    {
        mDrawMapLayers[ilayer] = true;
    }
}

void GameCheatsWindow::DoUI(Timespan deltaTime)
{
    ImGuiWindowFlags wndFlags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | 
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;

    ImGuiIO& io = ImGui::GetIO();
    if (!ImGui::Begin(mWindowName, &mWindowShown, wndFlags))
    {
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Frame Time: %.3f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Block chunks drawn: %d", gRenderManager.mMapRenderer.mRenderStats.mBlockChunksDrawnCount);
    
    // pedestrian stats
    if (Pedestrian* pedestrian = gCarnageGame.mHumanCharacters[0].mCharPedestrian)
    {
        ImGui::Separator();
        glm::vec3 pedPosition = pedestrian->mPhysicsComponent->GetPosition();
        ImGui::Text("pos: %f, %f, %f", pedPosition.x, pedPosition.y, pedPosition.z);

        cxx::angle_t pedHeading = pedestrian->mPhysicsComponent->GetRotationAngle();
        ImGui::Text("heading: %f", pedHeading.mDegrees);
        ImGui::Text("weapon: %s", cxx::enum_to_string(pedestrian->mCurrentWeapon));
        ImGui::Text("state: %s", cxx::enum_to_string(pedestrian->GetCurrentStateID()));
        ImGui::Separator();

        // get block location
        int mapcoordx = (int) pedPosition.x;
        int mapcoordy = (int) pedPosition.z;
        int maplayer = (int) pedPosition.y;

        BlockStyle* currBlock = gGameMap.GetBlockClamp(mapcoordx, mapcoordy, maplayer);

        ImGui::Text("b ground: %s", cxx::enum_to_string(currBlock->mGroundType));
        ImGui::Text("b slope: %d", currBlock->mSlopeType);
        ImGui::Text("b directions: %d, %d, %d, %d", currBlock->mUpDirection, currBlock->mRightDirection, 
            currBlock->mDownDirection, currBlock->mLeftDirection);

        ImGui::Separator();
        ImGui::SliderInt("ped remap", &pedestrian->mRemapIndex, -1, MAX_PED_REMAPS - 1);

        if (pedestrian->IsCarPassenger())
        {
            ImGui::SliderInt("car remap", &pedestrian->mCurrentCar->mRemapIndex, -1, MAX_CAR_REMAPS - 1);
        }
    }

    const char* modeStrings[] = { "Follow", "Free Look" };
    CameraController* modeControllers[] =
    {
        &gCarnageGame.mHumanCharacters[0].mCharView.mFollowCameraController, 
        &gCarnageGame.mHumanCharacters[0].mCharView.mFreeLookCameraController,
    }; 
    int currentCameraMode = 0;
    for (int i = 0; i < IM_ARRAYSIZE(modeControllers); ++i)
    {
        if (gCarnageGame.mHumanCharacters[0].mCharView.mCameraController == modeControllers[i])
        {
            currentCameraMode = i;
            break;
        }
    }
    const char* item_current = modeStrings[currentCameraMode];
    if (ImGui::BeginCombo("Camera mode", item_current))
    {
        for (int n = 0; n < IM_ARRAYSIZE(modeStrings); n++)
        {
            bool is_selected = (item_current == modeStrings[n]);
            if (ImGui::Selectable(modeStrings[n], is_selected))
            {
                item_current = modeStrings[n];
                gCarnageGame.mHumanCharacters[0].mCharView.SetCameraController(modeControllers[n]);
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::CollapsingHeader("Physics"))
    {
        ImGui::Checkbox("Enable map collisions", &mEnableMapCollisions);
        ImGui::Checkbox("Enable gravity", &mEnableGravity);
        ImGui::Separator();
    }

    if (ImGui::CollapsingHeader("Map Draw"))
    {
        ImGui::Checkbox("Enable blocks animation", &mEnableBlocksAnimation);
    }

    if (ImGui::CollapsingHeader("Ped"))
    {
        ImGui::SliderFloat("Turn speed", &gGameParams.mPedestrianTurnSpeed, 10.0f, 640.0f, "%.2f");
        ImGui::SliderFloat("Run speed", &gGameParams.mPedestrianRunSpeed, 0.1f, 16.0f, "%.2f");
        ImGui::SliderFloat("Walk speed", &gGameParams.mPedestrianWalkSpeed, 0.1f, 16.0f, "%.2f");
    }

    if (ImGui::CollapsingHeader("Graphics"))
    {
        if (ImGui::Checkbox("Enable vsync", &gSystem.mConfig.mEnableVSync))
        {
            gGraphicsDevice.EnableVSync(gSystem.mConfig.mEnableVSync);
        }
        if (ImGui::Checkbox("Fullscreen", &gSystem.mConfig.mFullscreen))
        {
            gGraphicsDevice.EnableFullscreen(gSystem.mConfig.mFullscreen);
            gGraphicsDevice.EnableVSync(gSystem.mConfig.mEnableVSync); // set vsync param as fullcreen mode changes
        }
    }

    ImGui::End();
}