#include "stdafx.h"
#include "RenderSystem.h"
#include "GpuTexture2D.h"
#include "GpuProgram.h"

RenderSystem gRenderSystem;

RenderSystem::RenderSystem()
    : mDefaultTexColorProgram("shaders/texture_color.glsl")
    , mGuiTexColorProgram("shaders/gui.glsl")
{
}

bool RenderSystem::Initialize()
{
    if (!InitRenderPrograms())
    {
        Deinit();
        return false;
    }

    if (!mGuiVertexCache.Initialize())
    {
        Deinit();
        return false;
    }

    return true;
}

void RenderSystem::Deinit()
{
    mGuiVertexCache.Deinit();

    FreeRenderPrograms();
}

void RenderSystem::RenderFrame()
{
    gGraphicsDevice.ClearScreen();

    // todo

    mDefaultTexColorProgram.Activate();

    gCamera.ComputeMatricesAndFrustum();

    mDefaultTexColorProgram.UploadCameraTransformMatrices();

    // todo

    gGuiSystem.RenderFrame();

    mGuiVertexCache.FlushCache();
    gGraphicsDevice.Present();
}

void RenderSystem::FreeRenderPrograms()
{
    mDefaultTexColorProgram.Deinit();
    mGuiTexColorProgram.Deinit();
}

bool RenderSystem::InitRenderPrograms()
{
    mDefaultTexColorProgram.Initialize();
    mGuiTexColorProgram.Initialize();

    return true;
}

void RenderSystem::ReloadRenderPrograms()
{
    mDefaultTexColorProgram.Reinitialize();
    mGuiTexColorProgram.Reinitialize();
}