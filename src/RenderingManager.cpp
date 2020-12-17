#include "stdafx.h"
#include "RenderingManager.h"
#include "GpuTexture2D.h"
#include "GpuProgram.h"
#include "SpriteManager.h"
#include "RenderView.h"
#include "GameCheatsWindow.h"
#include "AiManager.h"
#include "TrafficManager.h"
#include "ParticleEffectsManager.h"
#include "ParticleRenderdata.h"

RenderingManager gRenderManager;

RenderingManager::RenderingManager()
    : mDefaultTexColorProgram("shaders/texture_color.glsl")
    , mGuiTexColorProgram("shaders/gui.glsl")
    , mCityMeshProgram("shaders/city_mesh.glsl")
    , mSpritesProgram("shaders/sprites.glsl")
    , mDebugProgram("shaders/debug.glsl")
    , mParticleProgram("shaders/particle.glsl")
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

    gGraphicsDevice.SetClearColor(Color32_SkyBlue);

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
    mMapRenderer.RenderFrameBegin();

    Rect viewportRectangle = gGraphicsDevice.mViewportRect;
    for (RenderView* currRenderview: mActiveRenderViews)
    {
        currRenderview->DrawFrameBegin();
        mMapRenderer.RenderFrame(currRenderview);
        RenderParticleEffects(currRenderview);

        // draw debug info for first human view only
        if (currRenderview == mActiveRenderViews[0] && gGameCheatsWindow.mEnableDebugDraw)
        {
            mDebugRenderer.RenderFrameBegin(currRenderview);
            mMapRenderer.DebugDraw(mDebugRenderer);
            gTrafficManager.DebugDraw(mDebugRenderer);
            gAiManager.DebugDraw(mDebugRenderer);
            gParticleManager.DebugDraw(mDebugRenderer);
            mDebugRenderer.RenderFrameEnd();
        }
    }
    gGraphicsDevice.SetViewportRect(viewportRectangle);

    gGuiManager.RenderFrame();

    for (RenderView* currRenderview: mActiveRenderViews)
    {
        currRenderview->DrawFrameEnd();
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
    mParticleProgram.Deinit();
    mDebugProgram.Deinit();
}

bool RenderingManager::InitRenderPrograms()
{
    mDefaultTexColorProgram.Initialize();
    mGuiTexColorProgram.Initialize();
    mCityMeshProgram.Initialize(); 
    mSpritesProgram.Initialize();
    mParticleProgram.Initialize();
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
    mParticleProgram.Reinitialize();
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

void RenderingManager::RegisterParticleEffect(ParticleEffect* particleEffect)
{
    debug_assert(particleEffect);

    if (particleEffect == nullptr)
        return;

    if (particleEffect->mRenderdata)
    {
        debug_assert(false);
        return;
    }

    ParticleRenderdata* renderdata = new ParticleRenderdata;
    particleEffect->SetRenderdata(renderdata);
}

void RenderingManager::UnregisterParticleEffect(ParticleEffect* particleEffect)
{
    debug_assert(particleEffect);

    if (particleEffect == nullptr)
        return;

    ParticleRenderdata* renderdata = particleEffect->mRenderdata;
    if (renderdata)
    {
        renderdata->DestroyVertexbuffer();
        delete renderdata;
    }
    particleEffect->SetRenderdata(nullptr);
}

void RenderingManager::RenderParticleEffects(RenderView* renderview)
{
    bool hasActiveParticleEffects = false;
    // check if there is something to draw
    for (ParticleEffect* currEffect: gParticleManager.mParticleEffects)
    {
        if (currEffect->IsEffectActive())
        {
            hasActiveParticleEffects = true;
            break;
        }
    }

    if (!hasActiveParticleEffects)
        return;

    debug_assert(renderview);

    mParticleProgram.Activate();
    mParticleProgram.UploadCameraTransformMatrices(renderview->mCamera);

    RenderStates renderStates = RenderStates()
        .Enable(RenderStateFlags_AlphaBlend)
        .Disable(RenderStateFlags_FaceCulling);
    gGraphicsDevice.SetRenderStates(renderStates);

    for (ParticleEffect* currEffect: gParticleManager.mParticleEffects)
    {
        if (currEffect->IsEffectInactive())
            continue;

        RenderParticleEffect(renderview, currEffect);
    }

    mParticleProgram.Deactivate();
}

void RenderingManager::RenderParticleEffect(RenderView* renderview, ParticleEffect* particleEffect)
{
    ParticleRenderdata* renderdata = particleEffect->mRenderdata;
    if (renderdata == nullptr)
    {
        debug_assert(false); // effect is not registered - something is wrong
        return; 
    }

    const int NumParticles = particleEffect->mAliveParticlesCount;
    if (NumParticles == 0)
        return;

    // update vertices
    if (renderdata->mIsInvalidated)
    {
        renderdata->ResetInvalidated();
        if (!renderdata->PrepareVertexbuffer(particleEffect->mParticles.size() * Sizeof_ParticleVertex))
        {
            debug_assert(false);
            return;
        }

        GpuBuffer* vertexbuffer = renderdata->mVertexBuffer;
        debug_assert(vertexbuffer);
        ParticleVertex* vertices = vertexbuffer->LockData<ParticleVertex>(BufferAccess_UnsynchronizedWrite | BufferAccess_InvalidateBuffer);
        if (vertices == nullptr)
        {
            debug_assert(false);
            return;
        }

        for (int icurrParticle = 0; icurrParticle < NumParticles; ++icurrParticle)
        {
            ParticleVertex& particleVertex = vertices[icurrParticle];
            const Particle& srcParticle = particleEffect->mParticles[icurrParticle];
            particleVertex.mPositionSize.x = srcParticle.mPosition.x;
            particleVertex.mPositionSize.y = srcParticle.mPosition.y;
            particleVertex.mPositionSize.z = srcParticle.mPosition.z;
            particleVertex.mPositionSize.w = srcParticle.mSize;
            particleVertex.mColor = srcParticle.mColor;
        }

        if (!vertexbuffer->Unlock())
        {
            debug_assert(false);
            return;
        }
    }

    if (renderdata->mVertexBuffer == nullptr)
    {
        debug_assert(false);
        return;
    }

    ParticleVertex_Format vFormat;
    gGraphicsDevice.BindVertexBuffer(renderdata->mVertexBuffer, vFormat);
    gGraphicsDevice.RenderPrimitives(ePrimitiveType_Points, 0, NumParticles);
}