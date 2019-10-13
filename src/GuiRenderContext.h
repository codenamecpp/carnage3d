#pragma once

#include "GraphicsDefs.h"
#include "StreamingVertexCache.h"

// defines gui render context
class GuiRenderContext final: public cxx::noncopyable
{
public:
    Rect2D mScreenViewport;

public:
    // setup render context internal state and allocate resources
    // @param screenViewport: gui viewport area
    // returns false on error
    bool Initialize(const Rect2D& screenViewport);

    // free allocated resources
    void Deinit();

    // prepare to rendering gui, should be called before using any of context methods
    void RenderFrameBegin();

    // flush any pending draw operations, should be called at end of frame
    void RenderFrameEnd();

    // change gui viewport area
    // @param screenViewport: new viewport area
    void SetScreenViewport(const Rect2D& screenViewport);

private:
    glm::mat4 mOrthoProjectionMatrix;
};