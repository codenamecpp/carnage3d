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

    struct TextureUnitState
    {
        // note: mutual exclusion is used for different texture types
        GpuBufferTexture* mBufferTexture = nullptr;
        GpuTexture2D* mTexture2D = nullptr;
        GpuTextureArray2D* mTextureArray2D = nullptr;
    };

    GpuVertexArrayHandle mVaoHandle;
    GpuBuffer* mCurrentBuffers[eBufferContent_COUNT];
    GpuProgram* mCurrentProgram;
    eTextureUnit mCurrentTextureUnit;
    TextureUnitState mCurrentTextures[eTextureUnit_COUNT];
};