#pragma once

#include "GraphicsDefs.h"

// master render system, it is intended to manage rendering pipeline of the game
class RenderSystem final: public cxx::noncopyable
{
public:
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
    void LoadRenderPrograms();
    void FreeRenderPrograms();

private:

};

extern RenderSystem gRenderSystem;