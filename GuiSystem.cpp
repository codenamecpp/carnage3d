#include "stdafx.h"
#include "GuiSystem.h"
#include "ImGuiManager.h"

GuiSystem gGuiSystem;

bool GuiSystem::Initialize()
{
    gImGuiManager.Initialize();

    return true;
}

void GuiSystem::Deinit()
{
    gImGuiManager.Deinit();
}

void GuiSystem::RenderFrame()
{
    gImGuiManager.RenderFrame();
}

void GuiSystem::UpdateFrame(Timespan deltaTime)
{
    gImGuiManager.UpdateFrame(deltaTime);
}

void GuiSystem::HandleEvent(MouseMovedInputEvent& inputEvent)
{
    gImGuiManager.HandleEvent(inputEvent);
}

void GuiSystem::HandleEvent(MouseScrollInputEvent& inputEvent)
{
    gImGuiManager.HandleEvent(inputEvent);
}

void GuiSystem::HandleEvent(MouseButtonInputEvent& inputEvent)
{
    gImGuiManager.HandleEvent(inputEvent);
}

void GuiSystem::HandleEvent(KeyInputEvent& inputEvent)
{
    gImGuiManager.HandleEvent(inputEvent);
}

void GuiSystem::HandleEvent(KeyCharEvent& inputEvent)
{
    gImGuiManager.HandleEvent(inputEvent);
}