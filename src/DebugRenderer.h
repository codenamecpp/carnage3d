#pragma once

#include "GraphicsDefs.h"
#include "TrimeshBuffer.h"

class RenderView;

// renders debug information
class DebugRenderer final: public cxx::noncopyable
{
public:
    DebugRenderer();
    bool Initialize();
    void Deinit();

    void RenderFrameBegin(RenderView* renderview);
    void RenderFrameEnd();

    void DrawLine(const glm::vec3& point_a, const glm::vec3& point_b, unsigned int line_color);
    void DrawCube(const glm::vec3& point_center, const glm::vec3& cube_dimensions, unsigned int line_color);
    void DrawSphere(const glm::vec3& point_center, float radius, unsigned int line_color);

private:
    void FlushPrimitives();

private:
    GpuBuffer* mVerticesBuffer = nullptr;
    Vertex3D_Debug* mVerticesPointer = nullptr;

    const unsigned int mMaxVerticesCount = 4096;
    unsigned int mCurrVerticesCount = 0;

    RenderView* mCurrentRenderView = nullptr;
};