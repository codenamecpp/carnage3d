#include "stdafx.h"
#include "GameCheatsWindow.h"
#include "imgui.h"

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