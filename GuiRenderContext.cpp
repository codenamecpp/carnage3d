#include "stdafx.h"
#include "GuiRenderContext.h"
#include "RenderManager.h"
#include "GpuProgram.h"

bool GuiRenderContext::Initialize(const Rect2D& screenViewport)
{
    SetScreenViewport(screenViewport);

    if (!mGuiVertexCache.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot init gui vertex cache");
        return false;
    }

    return true;
}

void GuiRenderContext::Deinit()
{
    mGuiVertexCache.Deinit();
}

void GuiRenderContext::SetScreenViewport(const Rect2D& screenViewport)
{
    if (mScreenViewport == screenViewport)
        return;

    mOrthoProjectionMatrix = glm::ortho(screenViewport.x * 1.0f, 
        (screenViewport.x + screenViewport.w) * 1.0f, 
        (screenViewport.y + screenViewport.h) * 1.0f, 
        screenViewport.y * 1.0f);

    mScreenViewport = screenViewport;
}

void GuiRenderContext::RenderFrameBegin()
{
    // setup gui specific render program
    gRenderManager.mGuiTexColorProgram.Activate();
    gRenderManager.mGuiTexColorProgram.mGpuProgram->SetUniform(eRenderUniform_ViewProjectionMatrix, mOrthoProjectionMatrix);
    
    // setup render states
    RenderStates guiRenderStates = RenderStates()
        .Disable(RenderStateFlags_FaceCulling)
        .Disable(RenderStateFlags_DepthTest)
        .SetAlphaBlend(eBlendMode_Alpha);

    gGraphicsDevice.SetRenderStates(guiRenderStates);
    gGraphicsDevice.SetScissorRect(mScreenViewport);
}

void GuiRenderContext::RenderFrameEnd()
{
    gGraphicsDevice.SetScissorRect(mScreenViewport);
    mGuiVertexCache.FlushCache();
}
