#pragma once

#include "GraphicsDefs.h"

// defines hardware 1d texture object
class GpuTexture1D final: public cxx::noncopyable
{
public:
    // public for convenience, don't change these fields directly
    GpuTextureHandle mResourceHandle;
    eTextureFilterMode mFiltering; // always nearest
    eTextureWrapMode mRepeating;
    Size2D mSize;
    eTextureFormat mFormat;

public:
    GpuTexture1D(GraphicsContext& graphicsContext);
    ~GpuTexture1D();

    // Create texture of specified format and upload pixels data, no mipmaps
    // @param textureFormat: Format
    // @param sizex: Texture dimensions, must be POT!
    // @param sourceData: Source data buffer
    bool Setup(eTextureFormat textureFormat, int sizex, const void* sourceData);

    // Uploads pixels data for first mipmap, size of source bitmap should match current texture dimensions
    // @param sourceData: Source data buffer
    bool Upload(const void* sourceData);

    // Set texture filter and wrap parameters
    // @param filtering: Filtering mode
    // @param repeating: Addressing mode
    void SetSamplerState(eTextureFilterMode filtering, eTextureWrapMode repeating);

    // Test whether texture is currently bound at specified texture unit
    // @param unitIndex: Index of texture unit
    bool IsTextureBound(eTextureUnit textureUnit) const;
    bool IsTextureBound() const;

    // Test whether texture is created
    bool IsTextureInited() const;

private:
    void SetSamplerStateImpl(eTextureFilterMode filtering, eTextureWrapMode repeating);
    void SetUnbound();

private:
    GraphicsContext& mGraphicsContext;
};