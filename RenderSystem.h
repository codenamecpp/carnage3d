#pragma once

#include "GraphicsDefs.h"
#include "RenderProgram.h"

// master render system, it is intended to manage rendering pipeline of the game
class RenderSystem final: public cxx::noncopyable
{
public:
    RenderProgram mDefaultTexColorProgram;

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

private:
    GpuTexture2D* mDummyTexture = nullptr;
    GpuBuffer* mDummyVertexBuffer = nullptr;
    GpuBuffer* mDummyIndexBuffer = nullptr;
};

extern RenderSystem gRenderSystem;