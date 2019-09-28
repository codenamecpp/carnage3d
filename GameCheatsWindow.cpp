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
    , mGenerateFullMeshForMap()
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
    
    // pedestrian stats
    if (Pedestrian* pedestrian = gCarnageGame.mPlayerPedestrian)
    {
        ImGui::Separator();
        glm::vec3 pedPosition = pedestrian->mPhysicsComponent->GetPosition();
        ImGui::Text("pos: %f, %f, %f", pedPosition.x, pedPosition.y, pedPosition.z);

        float pedHeading = pedestrian->mPhysicsComponent->GetAngleDegrees();
        ImGui::Text("heading: %f", pedHeading);
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
        //ImGui::Text("b flat: %d", currBlock->mIsFlat);
        //for (int iface = 0; iface < eBlockFace_COUNT; ++iface)
        //{
        //    ImGui::Text("b face %s : %d", cxx::enum_to_string((eBlockFace) iface), currBlock->mFaces[iface]);
        //}
    }

    const char* modeStrings[] = { "Follow", "Free Look" };
    CameraController* modeControllers[] =
    {
        &gCarnageGame.mFollowCameraController, 
        &gCarnageGame.mFreeLookCameraController,
    }; 
    int currentCameraMode = 0;
    for (int i = 0; i < IM_ARRAYSIZE(modeControllers); ++i)
    {
        if (gCarnageGame.mCameraController == modeControllers[i])
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
                gCarnageGame.SetCameraController(modeControllers[n]);
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
        for (int ilayer = 0; ilayer < MAP_LAYERS_COUNT; ++ilayer)
        {
            cxx::string_buffer_16 cbtext;
            cbtext.printf("layer %d", ilayer);
            ImGui::Checkbox(cbtext.c_str(), &mDrawMapLayers[ilayer]);
        }
        ImGui::Separator();
        if (ImGui::Checkbox("Generate full mesh for map", &mGenerateFullMeshForMap))
        {
            gRenderManager.mCityRenderer.InvalidateMapMesh();
        }
        ImGui::Separator();
        ImGui::Checkbox("Enable blocks animation", &mEnableBlocksAnimation);
    }

    if (ImGui::CollapsingHeader("Ped"))
    {
        ImGui::SliderFloat("Turn speed", &gGameRules.mPedestrianTurnSpeed, 10.0f, 640.0f, "%.2f");
        ImGui::SliderFloat("Run speed", &gGameRules.mPedestrianRunSpeed, 0.1f, 16.0f, "%.2f");
        ImGui::SliderFloat("Walk speed", &gGameRules.mPedestrianWalkSpeed, 0.1f, 16.0f, "%.2f");
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
        }
    }

    ImGui::End();
}