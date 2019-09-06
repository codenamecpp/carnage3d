#include "stdafx.h"
#include "GameCheatsWindow.h"
#include "imgui.h"
#include "RenderingManager.h"

GameCheatsWindow gGameCheatsWindow;

GameCheatsWindow::GameCheatsWindow()
    : DebugWindow("Game Cheats")
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
    
    // pedestrian stats
    if (Pedestrian* pedestrian = gCarnageGame.mPlayerPedestrian)
    {
        ImGui::Separator();
        ImGui::Text("pos: %f, %f, %f", pedestrian->mPosition.x, pedestrian->mPosition.y, pedestrian->mPosition.z);
        ImGui::Text("heading: %f", pedestrian->mHeading);
        ImGui::Separator();

        int curr_bloc_x = (int) pedestrian->mPosition.x;
        int curr_bloc_y = (int) pedestrian->mPosition.y;
        int curr_bloc_z = (int) pedestrian->mPosition.z + 2;

        BlockStyleData* currBlock = gMapManager.GetBlockClamp(curr_bloc_x, curr_bloc_y, curr_bloc_z);
        debug_assert(currBlock);

        ImGui::Text("b ground: %s", cxx::enum_to_string(currBlock->mGroundType));
        ImGui::Text("b slope: %d", currBlock->mSlopeType);
        ImGui::Text("b directions: %d, %d, %d, %d", currBlock->mUpDirection, currBlock->mRightDirection, 
            currBlock->mDownDirection, currBlock->mLeftDirection);
        ImGui::Text("b flat: %d", currBlock->mIsFlat);
        ImGui::Text("b flip top-bottom: %d", currBlock->mFlipTopBottomFaces);
        ImGui::Text("b flip left-right: %d", currBlock->mFlipLeftRightFaces);

        // draw debug block

        glm::vec3 cube_center { 
            MAP_BLOCK_LENGTH * curr_bloc_x + MAP_BLOCK_LENGTH * 0.5f,
            MAP_BLOCK_LENGTH * curr_bloc_z + MAP_BLOCK_LENGTH * 0.5f + 0.05f,
            MAP_BLOCK_LENGTH * curr_bloc_y + MAP_BLOCK_LENGTH * 0.5f,
        };
        glm::vec3 cube_dims {
            MAP_BLOCK_LENGTH - 0.1f,
            MAP_BLOCK_LENGTH - 0.1f,
            MAP_BLOCK_LENGTH - 0.1f
        };  

        gRenderManager.mDebugRenderer.DrawCube(cube_center, cube_dims, COLOR_GREEN);
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Map layers"))
    {
        for (int ilayer = 0; ilayer < MAP_LAYERS_COUNT; ++ilayer)
        {
            cxx::string_buffer_16 cbtext;
            cbtext.printf("layer %d", ilayer);
            ImGui::Checkbox(cbtext.c_str(), &mDrawMapLayers[ilayer]);
        }
    }

    if (ImGui::CollapsingHeader("Ped"))
    {
        ImGui::SliderFloat("Turn speed", &gGameRules.mPedestrianTurnSpeed, 10.0f, 640.0f, "%.2f");
        ImGui::SliderFloat("Run speed", &gGameRules.mPedestrianRunSpeed, 0.1f, 16.0f, "%.2f");
        ImGui::SliderFloat("Walk speed", &gGameRules.mPedestrianWalkSpeed, 0.1f, 16.0f, "%.2f");
    }

    ImGui::End();
}