#pragma once

#include "GraphicsDefs.h"

// defines hardware 2d texture object
class GpuTexture final: public cxx::noncopyable
{
public:
    // public for convenience, don't change these fields directly
    GpuTextureHandle mResourceHandle;
    eTextureFilterMode mFiltering;
    eTextureWrapMode mRepeating;
    Size2D mSize;
    eTextureFormat mFormat;

public:
    GpuTexture(GraphicsContext& graphicsContext);
    ~GpuTexture();

    // Create texture of specified format and upload pixels data, no mipmaps
    // @param textureFormat: Format
    // @param sizex, sizey: Texture dimensions, must be POT!
    // @param sourceData: Source data buffer
    bool Setup(eTextureFormat textureFormat, int sizex, int sizey, const void* sourceData);

    // Free texture data but dont destroy hardware object
    void Invalidate();

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