#include "stdafx.h"
#include "ParticleRenderdata.h"

void ParticleRenderdata::Invalidate()
{
    mIsInvalidated = true;
}

bool ParticleRenderdata::PrepareVertexbuffer(int vertexbufferSize)
{
    if (vertexbufferSize == 0)
        return true;

    if (mVertexBuffer == nullptr)
    {
        mVertexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices, eBufferUsage_Stream, vertexbufferSize, nullptr);
        if (mVertexBuffer == nullptr)
            return false;

        return true;
    }

    return mVertexBuffer->Setup(eBufferUsage_Stream, vertexbufferSize, nullptr);
}

void ParticleRenderdata::DestroyVertexbuffer()
{
    if (mVertexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mVertexBuffer);
        mVertexBuffer = nullptr;
    }
}

void ParticleRenderdata::ResetInvalidated()
{
    mIsInvalidated = false;
}
