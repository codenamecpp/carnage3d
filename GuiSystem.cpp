#include "stdafx.h"
#include "GuiSystem.h"
#include "ImGuiManager.h"

GuiSystem gGuiSystem;

bool GuiSystem::Initialize()
{
    if (!mRenderContext.Initialize(gGraphicsDevice.mViewportRect))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize gui render context");
        return false;
    }

    gImGuiManager.Initialize();
    return true;
}

void GuiSystem::Deinit()
{
    gImGuiManager.Deinit();

    mRenderContext.Deinit();
}

void GuiSystem::RenderFrame()
{
    mRenderContext.RenderFrameBegin();

    // draw imgui debug ui
    gImGuiManager.RenderFrame(mRenderContext);

    mRenderContext.RenderFrameEnd();
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

void GuiSystem::SetShowDebugConsole(bool isShown)
{
    gImGuiManager.mConsoleWindow.mShown = isShown;
}
