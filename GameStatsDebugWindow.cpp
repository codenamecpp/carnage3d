#include "stdafx.h"
#include "GameStatsDebugWindow.h"
#include "imgui.h"

GameStatsDebugWindow gGameStatsDebugWindow;

GameStatsDebugWindow::GameStatsDebugWindow()
    : DebugWindow("Game Stats")
{
}

void GameStatsDebugWindow::DoUI(Timespan deltaTime)
{
    ImGuiWindowFlags wndFlags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | 
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;

    ImGuiIO& io = ImGui::GetIO();
    if (!ImGui::Begin(mWindowName, &mWindowShown, wndFlags))
    {
        ImGui::End();
        return;
    }
    ImGui::Separator();

    // pedestrian stats
    if (Pedestrian* pedestrian = gCarnageGame.mPlayerPedestrian)
    {
        ImGui::Text("pos: %f, %f, %f", pedestrian->mPosition.x, pedestrian->mPosition.y, pedestrian->mPosition.z);
        ImGui::Text("heading: %f", pedestrian->mHeading);
    }

    ImGui::End();
}