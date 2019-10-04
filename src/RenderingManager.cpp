#include "stdafx.h"
#include "RenderingManager.h"
#include "GpuTexture2D.h"
#include "GpuProgram.h"
#include "SpriteManager.h"

RenderingManager gRenderManager;

RenderingManager::RenderingManager()
    : mDefaultTexColorProgram("shaders/texture_color.glsl")
    , mGuiTexColorProgram("shaders/gui.glsl")
    , mCityMeshProgram("shaders/city_mesh.glsl")
    , mSpritesProgram("shaders/sprites.glsl")
    , mDebugProgram("shaders/debug.glsl")
{
}

bool RenderingManager::Initialize()
{
    if (!InitRenderPrograms())
    {
        Deinit();
        return false;
    }

    if (!mMapRenderer.Initialize())
    {
        Deinit();
        return false;
    }

    if (!mDebugRenderer.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize debug renderer");
    }

    gGraphicsDevice.SetClearColor(COLOR_SKYBLUE);

    return true;
}

void RenderingManager::Deinit()
{
    mDebugRenderer.Deinit();
    mMapRenderer.Deinit();
    gSpriteManager.Cleanup();
    FreeRenderPrograms();
}

void RenderingManager::RenderFrame()
{
    gGraphicsDevice.ClearScreen();
    gCamera.ComputeMatricesAndFrustum();
    gSpriteManager.RenderFrameBegin();
    mMapRenderer.RenderFrame();
    mDebugRenderer.RenderFrame();
    gGuiSystem.RenderFrame();
    gSpriteManager.RenderFrameEnd();
    gGraphicsDevice.Present();
}

void RenderingManager::FreeRenderPrograms()
{
    mDefaultTexColorProgram.Deinit();
    mCityMeshProgram.Deinit();
    mGuiTexColorProgram.Deinit();
    mSpritesProgram.Deinit();
    mDebugProgram.Deinit();
}

bool RenderingManager::InitRenderPrograms()
{
    if (!mDefaultTexColorProgram.Initialize() || !mGuiTexColorProgram.Initialize() ||
        !mCityMeshProgram.Initialize() || !mSpritesProgram.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize render programs");
        return false;
    }

    if (!mDebugProgram.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize render programs");
    }

    return true;
}

void RenderingManager::ReloadRenderPrograms()
{
    gConsole.LogMessage(eLogMessage_Info, "Reloading render programs...");

    mDefaultTexColorProgram.Reinitialize();
    mGuiTexColorProgram.Reinitialize();
    mDebugProgram.Reinitialize();
    mSpritesProgram.Reinitialize();
    mCityMeshProgram.Reinitialize();
}