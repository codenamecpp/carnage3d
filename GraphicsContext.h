#pragma once

#include "GraphicsDefs.h"

// GraphicsContext represents current graphics device state which is low-level and 
// does not intended for direct usage

class GraphicsContext
{
public:
    GraphicsContext() 
        : mCurrentBuffers()
        , mCurrentTextureUnit(eTextureUnit_0)
        , mCurrentTextures()
        , mCurrentProgram()
        , mVaoHandle()
    {
    }
public:
    GpuVertexArrayHandle mVaoHandle;
    GpuBuffer* mCurrentBuffers[eBufferContent_COUNT];
    GpuProgram* mCurrentProgram;
    eTextureUnit mCurrentTextureUnit;
    GpuTexture2D* mCurrentTextures[eTextureUnit_COUNT];
};