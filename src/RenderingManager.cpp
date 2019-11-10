#include "stdafx.h"
#include "RenderingManager.h"
#include "GpuTexture2D.h"
#include "GpuProgram.h"
#include "SpriteManager.h"
#include "RenderView.h"
#include "GameCheatsWindow.h"

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
    InitRenderPrograms();

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
    mActiveRenderViews.clear();
    mDebugRenderer.Deinit();
    mMapRenderer.Deinit();
    gSpriteManager.Cleanup();

    FreeRenderPrograms();
}

void RenderingManager::RenderFrame()
{
    gGraphicsDevice.ClearScreen();
    gSpriteManager.RenderFrameBegin();
    mMapRenderer.RenderFrameStart();

    Rect2D viewportRectangle = gGraphicsDevice.mViewportRect;
    for (RenderView* currRenderview: mActiveRenderViews)
    {
        currRenderview->PreRender();
        mMapRenderer.RenderFrame(currRenderview);

        // draw debug info for first human view only
        if (currRenderview == mActiveRenderViews[0] && gGameCheatsWindow.mEnableDebugDraw)
        {
            mDebugRenderer.RenderFrameBegin(currRenderview);
            mMapRenderer.RenderDebug(currRenderview, mDebugRenderer);
            mDebugRenderer.RenderFrameEnd();
        }
    }

    gGraphicsDevice.SetViewportRect(viewportRectangle);
    gGuiSystem.RenderFrame();

    for (RenderView* currRenderview: mActiveRenderViews)
    {
        currRenderview->PostRender();
    }
    mMapRenderer.RenderFrameEnd();
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
    mDefaultTexColorProgram.Initialize();
    mGuiTexColorProgram.Initialize();
    mCityMeshProgram.Initialize(); 
    mSpritesProgram.Initialize();
    mDebugProgram.Initialize();

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

void RenderingManager::AttachRenderView(RenderView* renderview)
{
    debug_assert(renderview);

    auto ifound = std::find(mActiveRenderViews.begin(), mActiveRenderViews.end(), renderview);
    if (ifound == mActiveRenderViews.end())
    {
        mActiveRenderViews.push_back(renderview);
        return;
    }
    debug_assert(false);
}

void RenderingManager::DetachRenderView(RenderView* renderview)
{
    debug_assert(renderview);

    auto ifound = std::find(mActiveRenderViews.begin(), mActiveRenderViews.end(), renderview);
    if (ifound != mActiveRenderViews.end())
    {
        mActiveRenderViews.erase(ifound);
        return;
    }
    debug_assert(false);
}