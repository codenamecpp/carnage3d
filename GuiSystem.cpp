#include "stdafx.h"
#include "GuiSystem.h"
#include "ImGuiRenderer.h"

GuiSystem gGuiSystem;

bool GuiSystem::Initialize()
{
    gImGuiRenderer.Initialize();

    return true;
}

void GuiSystem::Deinit()
{
    gImGuiRenderer.Deinit();
}

void GuiSystem::RenderFrame()
{
    gImGuiRenderer.RenderFrame();
}

void GuiSystem::UpdateFrame(Timespan deltaTime)
{
    gImGuiRenderer.UpdateFrame(deltaTime);
}

void GuiSystem::HandleEvent(MouseMovedInputEvent& inputEvent)
{
    gImGuiRenderer.HandleEvent(inputEvent);
}

void GuiSystem::HandleEvent(MouseScrollInputEvent& inputEvent)
{
    gImGuiRenderer.HandleEvent(inputEvent);
}

void GuiSystem::HandleEvent(MouseButtonInputEvent& inputEvent)
{
    gImGuiRenderer.HandleEvent(inputEvent);
}

void GuiSystem::HandleEvent(KeyInputEvent& inputEvent)
{
    gImGuiRenderer.HandleEvent(inputEvent);
}

void GuiSystem::HandleEvent(KeyCharEvent& inputEvent)
{
    gImGuiRenderer.HandleEvent(inputEvent);
}