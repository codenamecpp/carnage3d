#include "stdafx.h"
#include "RenderManager.h"
#include "GpuTexture2D.h"
#include "GpuProgram.h"
#include "SpriteCache.h"

RenderManager gRenderManager;

RenderManager::RenderManager()
    : mDefaultTexColorProgram("shaders/texture_color.glsl")
    , mGuiTexColorProgram("shaders/gui.glsl")
    , mCityMeshProgram("shaders/city_mesh.glsl")
    , mSpritesProgram("shaders/sprites.glsl")
{
}

bool RenderManager::Initialize()
{
    if (!InitRenderPrograms())
    {
        Deinit();
        return false;
    }

    if (!mCityRenderer.Initialize())
    {
        Deinit();
        return false;
    }

    return true;
}

void RenderManager::Deinit()
{
    mCityRenderer.Deinit();
    gSpriteCache.Cleanup();
    FreeRenderPrograms();
}

void RenderManager::RenderFrame()
{
    gGraphicsDevice.ClearScreen();
    gCamera.ComputeMatricesAndFrustum();

    //mDefaultTexColorProgram.Activate();
    //mDefaultTexColorProgram.UploadCameraTransformMatrices();
    mCityRenderer.RenderFrame();
    gGuiSystem.RenderFrame();
    gGraphicsDevice.Present();
}

void RenderManager::FreeRenderPrograms()
{
    mDefaultTexColorProgram.Deinit();
    mCityMeshProgram.Deinit();
    mGuiTexColorProgram.Deinit();
    mSpritesProgram.Deinit();
}

bool RenderManager::InitRenderPrograms()
{
    if (!mDefaultTexColorProgram.Initialize() || !mGuiTexColorProgram.Initialize() ||
        !mCityMeshProgram.Initialize() || !mSpritesProgram.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize render programs");
        return false;
    }

    return true;
}

void RenderManager::ReloadRenderPrograms()
{
    gConsole.LogMessage(eLogMessage_Info, "Reloading render programs...");

    mDefaultTexColorProgram.Reinitialize();
    mGuiTexColorProgram.Reinitialize();
    mSpritesProgram.Reinitialize();
    mCityMeshProgram.Reinitialize();
}