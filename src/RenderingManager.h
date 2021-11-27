#pragma once

#include "GraphicsDefs.h"
#include "RenderProgram.h"
#include "MapRenderer.h"
#include "DebugRenderer.h"
#include "ParticleEffect.h"

// master render system, it is intended to manage rendering pipeline of the game
class RenderingManager final: public cxx::noncopyable
{
public:
    RenderProgram mDefaultTexColorProgram;
    RenderProgram mCityMeshProgram;
    RenderProgram mGuiTexColorProgram;
    RenderProgram mSpritesProgram;
    RenderProgram mDebugProgram;
    RenderProgram mParticleProgram;

    MapRenderer mMapRenderer;

    std::vector<GameCamera*> mActiveRenderViews;

public:
    RenderingManager();

    // First time render system initialization
    // All shaders, buffers and other graphics resources might be loaded here
    // Return false on error
    bool Initialize();

    // System finalization
    // All loaded graphics resources must be destroyed here
    void Deinit();

    // Render game frame routine
    void RenderFrame();

    // Force reload all render programs
    void ReloadRenderPrograms();
    
    void AttachRenderView(GameCamera* renderview);
    void DetachRenderView(GameCamera* renderview);

    // Register particle effect for rendering
    void RegisterParticleEffect(ParticleEffect* particleEffect);
    void UnregisterParticleEffect(ParticleEffect* particleEffect);

private:
    void RenderParticleEffects(GameCamera* renderview);
    void RenderParticleEffect(GameCamera* renderview, ParticleEffect* particleEffect);

private:
    bool InitRenderPrograms();
    void FreeRenderPrograms();

private:
    DebugRenderer mDebugRenderer;
};

extern RenderingManager gRenderManager;