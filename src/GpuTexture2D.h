#pragma once

#include "GraphicsDefs.h"

// defines hardware 2d texture object
class GpuTexture2D final: public cxx::noncopyable
{
public:
    // public for convenience, don't change these fields directly
    GpuTextureHandle mResourceHandle;
    eTextureFilterMode mFiltering;
    eTextureWrapMode mRepeating;
    Size2D mSize;
    eTextureFormat mFormat;

public:
    GpuTexture2D(GraphicsContext& graphicsContext);
    ~GpuTexture2D();

    // Create texture of specified format and upload pixels data, no mipmaps
    // @param textureFormat: Format
    // @param sizex, sizey: Texture dimensions, must be POT!
    // @param sourceData: Source data buffer
    bool Setup(eTextureFormat textureFormat, int sizex, int sizey, const void* sourceData);

    // Uploads pixels data
    // @param mipLevel: Specifies the level-of-detail number; level 0 is the base image level
    // @param xoffset, yoffset: Specifies a texel offset within the texture array
    // @param sizex, sizey: Specifies the size of the texture subimage
    // @param sourceData: Specifies a pointer to the source data
    bool Upload(int mipLevel, int xoffset, int yoffset, int sizex, int sizey, const void* sourceData);
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