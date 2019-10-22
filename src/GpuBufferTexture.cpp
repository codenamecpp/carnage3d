#include "stdafx.h"
#include "GpuBufferTexture.h"
#include "GraphicsDevice.h"
#include "GraphicsContext.h"
#include "OpenGLDefs.h"

//////////////////////////////////////////////////////////////////////////

class ScopedBufferTextureBinder
{
public:
    ScopedBufferTextureBinder(GraphicsContext& renderContext, GpuBufferTexture* gpuTexture)
        : mRenderContext(renderContext)
        , mPreviousTexture(renderContext.mCurrentTextures[renderContext.mCurrentTextureUnit].mBufferTexture)
        , mTexture(gpuTexture)
    {
        debug_assert(gpuTexture);
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_BUFFER, mTexture->mResourceHandle);
            glCheckError();
        }
    }
    ~ScopedBufferTextureBinder()
    {
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_BUFFER, mPreviousTexture ? mPreviousTexture->mResourceHandle : 0);
            glCheckError();
        }
    }
private:
    GraphicsContext& mRenderContext;
    GpuBufferTexture* mPreviousTexture;
    GpuBufferTexture* mTexture;
};

//////////////////////////////////////////////////////////////////////////

GpuBufferTexture::GpuBufferTexture(GraphicsContext& renderContext)
    : mGraphicsContext(renderContext)
    , mResourceHandle()
    , mBufferHandle()
    , mFormat()
    , mBufferLength()
{
    ::glGenBuffers(1, &mBufferHandle);
    glCheckError();

    ::glGenTextures(1, &mResourceHandle);
    glCheckError();
}

GpuBufferTexture::~GpuBufferTexture()
{
    SetUnbound();

    ::glDeleteTextures(1, &mResourceHandle);
    glCheckError();

    ::glDeleteBuffers(1, &mBufferHandle);
    glCheckError();
}

bool GpuBufferTexture::Setup(eTextureFormat textureFormat, int dataLength, const void* sourceData)
{
    GLint internalFormatGL = GetTextureInternalFormatGL(textureFormat);
    if (internalFormatGL == 0)
    {
        debug_assert(false);
        return false;
    }

    mFormat = textureFormat;
    mBufferLength = dataLength;
    
    ScopedBufferTextureBinder scopedBind(mGraphicsContext, this);

	::glBindBuffer(GL_TEXTURE_BUFFER, mBufferHandle);
    glCheckError();

	::glBufferData(GL_TEXTURE_BUFFER, dataLength, sourceData, GL_DYNAMIC_DRAW);
    glCheckError();

    ::glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glCheckError();

    ::glTexBuffer(GL_TEXTURE_BUFFER, internalFormatGL, mBufferHandle);
    glCheckError();

    return true;
}


bool GpuBufferTexture::IsTextureBound(eTextureUnit textureUnit) const
{
    if (!IsTextureInited())
        return false;

    debug_assert(textureUnit < eTextureUnit_COUNT);
    return this == mGraphicsContext.mCurrentTextures[textureUnit].mBufferTexture;
}

bool GpuBufferTexture::IsTextureBound() const
{
    if (!IsTextureInited())
        return false;

    for (int itexunit = 0; itexunit < eTextureUnit_COUNT; ++itexunit)
    {
        if (this == mGraphicsContext.mCurrentTextures[itexunit].mBufferTexture)
            return true;
    }

    return false;
}

bool GpuBufferTexture::Upload(int dataOffset, int dataLength, const void* sourceData)
{
    if (!IsTextureInited())
        return false;

    debug_assert(sourceData);
    debug_assert(dataOffset + dataLength <= mBufferLength);

	::glBindBuffer(GL_TEXTURE_BUFFER, mBufferHandle);
    glCheckError();

	::glBufferSubData(GL_TEXTURE_BUFFER, dataOffset, dataLength, sourceData);
    glCheckError();

    ::glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glCheckError();
    return true;
}

bool GpuBufferTexture::IsTextureInited() const
{
    return mFormat != eTextureFormat_Null;
}

void GpuBufferTexture::SetUnbound()
{
    for (int iTextureUnit = 0; iTextureUnit < eTextureUnit_COUNT; ++iTextureUnit)
    {
        if (this == mGraphicsContext.mCurrentTextures[iTextureUnit].mBufferTexture)
        {
            mGraphicsContext.mCurrentTextures[iTextureUnit].mBufferTexture = nullptr;
        }
    }
}
