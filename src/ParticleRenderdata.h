#pragma once

#include "GpuBuffer.h"

// Renderdata is associated with particle effect instance
class ParticleRenderdata final: public cxx::noncopyable
{
    friend class RenderingManager;

public:
    void Invalidate();
    void ResetInvalidated();
private:
    bool PrepareVertexbuffer(int vertexbufferSize);
    void DestroyVertexbuffer();
private:
    GpuBuffer* mVertexBuffer = nullptr;
    bool mIsInvalidated = false;
};