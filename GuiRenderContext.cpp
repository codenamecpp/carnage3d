#include "stdafx.h"
#include "GuiRenderContext.h"
#include "RenderSystem.h"
#include "GpuProgram.h"

bool GuiRenderContext::Initialize(const Rect2D& screenViewport)
{
    SetScreenViewport(screenViewport);
    return true;
}

void GuiRenderContext::Deinit()
{
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
    gRenderSystem.mGuiTexColorProgram.Activate();
    gRenderSystem.mGuiTexColorProgram.mGpuProgram->SetUniform(eRenderUniform_ViewProjectionMatrix, mOrthoProjectionMatrix);
    
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

}
