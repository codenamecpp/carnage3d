#pragma once

#include "GraphicsDefs.h"
#include "RenderProgram.h"
#include "StreamingVertexCache.h"
#include "CityMapRenderer.h"
#include "PedestrianRenderer.h"

// master render system, it is intended to manage rendering pipeline of the game
class RenderSystem final: public cxx::noncopyable
{
public:
    RenderProgram mDefaultTexColorProgram;
    RenderProgram mCityMeshProgram;
    RenderProgram mGuiTexColorProgram;

    StreamingVertexCache mGuiVertexCache;
    StreamingVertexCache mMapObjectsVertexCache; // peds, cars and map objects

    CityMapRenderer mCityMapRenderer;
    PedestrianRenderer mPedestrianRenderer;

public:
    RenderSystem();

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

extern RenderSystem gRenderSystem;