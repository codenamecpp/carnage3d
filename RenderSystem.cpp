#include "stdafx.h"
#include "RenderSystem.h"
#include "GpuTexture2D.h"
#include "GpuProgram.h"
#include "SpriteCache.h"

RenderSystem gRenderSystem;

RenderSystem::RenderSystem()
    : mDefaultTexColorProgram("shaders/texture_color.glsl")
    , mGuiTexColorProgram("shaders/gui.glsl")
    , mCityMeshProgram("shaders/city_mesh.glsl")
{
}

bool RenderSystem::Initialize()
{
    if (!InitRenderPrograms())
    {
        Deinit();
        return false;
    }

    if (!mCityRenderer.Initialize() || !mGuiVertexCache.Initialize())
    {
        Deinit();
        return false;
    }

    return true;
}

void RenderSystem::Deinit()
{
    mCityRenderer.Deinit();
    mGuiVertexCache.Deinit();

    gSpriteCache.Cleanup();
    FreeRenderPrograms();
}

void RenderSystem::RenderFrame()
{
    gGraphicsDevice.ClearScreen();
    gCamera.ComputeMatricesAndFrustum();

    //mDefaultTexColorProgram.Activate();
    //mDefaultTexColorProgram.UploadCameraTransformMatrices();

    mCityRenderer.RenderFrame();

    gGuiSystem.RenderFrame();

    mGuiVertexCache.FlushCache();
    gGraphicsDevice.Present();
}

void RenderSystem::FreeRenderPrograms()
{
    mDefaultTexColorProgram.Deinit();
    mCityMeshProgram.Deinit();
    mGuiTexColorProgram.Deinit();
}

bool RenderSystem::InitRenderPrograms()
{
    if (!mDefaultTexColorProgram.Initialize() ||
        !mGuiTexColorProgram.Initialize() ||
        !mCityMeshProgram.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize render programs");
        return false;
    }

    return true;
}

void RenderSystem::ReloadRenderPrograms()
{
    mDefaultTexColorProgram.Reinitialize();
    mGuiTexColorProgram.Reinitialize();
    mCityMeshProgram.Reinitialize();
}