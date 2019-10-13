#pragma once

#include "GraphicsDefs.h"
#include "RenderProgram.h"
#include "StreamingVertexCache.h"
#include "MapRenderer.h"
#include "DebugRenderer.h"

// master render system, it is intended to manage rendering pipeline of the game
class RenderingManager final: public cxx::noncopyable
{
public:
    RenderProgram mDefaultTexColorProgram;
    RenderProgram mCityMeshProgram;
    RenderProgram mGuiTexColorProgram;
    RenderProgram mSpritesProgram;
    RenderProgram mDebugProgram;
    StreamingVertexCache mStreamingVertexCache;

    MapRenderer mMapRenderer;
    DebugRenderer mDebugRenderer;

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

private:
    bool InitRenderPrograms();
    void FreeRenderPrograms();
};

extern RenderingManager gRenderManager;