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

    for (int ilayer = 0; ilayer < MAP_LAYERS_COUNT; ++ilayer)
    {
        cxx::string_buffer_16 cbtext;
        cbtext.printf("map Layer %d", ilayer);
        ImGui::Checkbox(cbtext.c_str(), &mDrawMapLayers[ilayer]);
    }

    ImGui::End();
}