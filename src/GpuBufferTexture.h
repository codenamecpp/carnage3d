#pragma once

#include "GraphicsDefs.h"

// defines hardware buffer texture object
class GpuBufferTexture final: public cxx::noncopyable
{
public:
    // public for convenience, don't change these fields directly
    GpuTextureHandle mResourceHandle;
    GpuBufferHandle mBufferHandle;
    int mBufferLength;
    eTextureFormat mFormat;

public:
    GpuBufferTexture(GraphicsContext& graphicsContext);
    ~GpuBufferTexture();

    // Create texture of specified format and upload pixels data
    // @param textureFormat: Format
    // @param dataLength: Data length in bytes
    // @param sourceData: Source data buffer
    bool Setup(eTextureFormat textureFormat, int dataLength, const void* sourceData);

    // Uploads data, size of source data should match specified buffer length
    // @param dataOffset: Buffer storage offset in bytes
    // @param dataLength: Data length in bytes
    // @param sourceData: Source data buffer
    bool Upload(int dataOffset, int dataLength, const void* sourceData);

    // Test whether texture is currently bound at specified texture unit
    // @param unitIndex: Index of texture unit
    bool IsTextureBound(eTextureUnit textureUnit) const;
    bool IsTextureBound() const;

    // Test whether texture is created
    bool IsTextureInited() const;

private:
    void SetUnbound();

private:
    GraphicsContext& mGraphicsContext;
};