#include "stdafx.h"
#include "GpuTexture1D.h"
#include "GraphicsDevice.h"
#include "GraphicsContext.h"
#include "OpenGLDefs.h"

//////////////////////////////////////////////////////////////////////////

class ScopedTexture1DBinder
{
public:
    ScopedTexture1DBinder(GraphicsContext& renderContext, GpuTexture1D* gpuTexture)
        : mRenderContext(renderContext)
        , mPreviousTexture(renderContext.mCurrentTextures[renderContext.mCurrentTextureUnit].mTexture1D)
        , mTexture(gpuTexture)
    {
        debug_assert(gpuTexture);
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_1D, mTexture->mResourceHandle);
            glCheckError();
        }
    }
    ~ScopedTexture1DBinder()
    {
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_1D, mPreviousTexture ? mPreviousTexture->mResourceHandle : 0);
            glCheckError();
        }
    }
private:
    GraphicsContext& mRenderContext;
    GpuTexture1D* mPreviousTexture;
    GpuTexture1D* mTexture;
};

//////////////////////////////////////////////////////////////////////////

GpuTexture1D::GpuTexture1D(GraphicsContext& renderContext)
    : mGraphicsContext(renderContext)
    , mResourceHandle()
    , mFiltering()
    , mRepeating()
    , mSize()
    , mFormat()
{
    ::glGenTextures(1, &mResourceHandle);
    glCheckError();
}

GpuTexture1D::~GpuTexture1D()
{
    SetUnbound();

    ::glDeleteTextures(1, &mResourceHandle);
    glCheckError();
}

bool GpuTexture1D::Setup(eTextureFormat textureFormat, int sizex, const void* sourceData)
{
    GLuint formatGL = 0;
    GLint internalFormatGL = 0;
    switch (textureFormat)
    {
        case eTextureFormat_R8: 
            formatGL = GL_RED;
            internalFormatGL = GL_R8;
        break;
        case eTextureFormat_R8_G8: 
            formatGL = GL_RG;
            internalFormatGL = GL_RG8;
        break;
        case eTextureFormat_RGB8: 
            formatGL = GL_RGB;
            internalFormatGL = GL_RGB8;
        break;
        case eTextureFormat_RGBA8: 
            formatGL = GL_RGBA;
            internalFormatGL = GL_RGBA8;
        break;
    }
    // unknown format
    if (formatGL == 0 || internalFormatGL == 0)
    {
        debug_assert(false);
        return false;
    }

    mFormat = textureFormat;
    mSize.x = sizex;
    mSize.y = 1;
    
    ScopedTexture1DBinder scopedBind(mGraphicsContext, this);
    ::glTexImage1D(GL_TEXTURE_1D, 0, internalFormatGL, mSize.x, 0, formatGL, GL_UNSIGNED_BYTE, sourceData);
    glCheckError();

    // set default filter and repeat mode for texture
    SetSamplerStateImpl(gGraphicsDevice.mDefaultTextureFilter, gGraphicsDevice.mDefaultTextureWrap);
    return true;
}

void GpuTexture1D::SetSamplerState(eTextureFilterMode filtering, eTextureWrapMode repeating)
{
    if (!IsTextureInited())
    {
        debug_assert(false);
        return;
    }

    // already set
    if (mFiltering == filtering && mRepeating == repeating)
        return;

    ScopedTexture1DBinder scopedBind(mGraphicsContext, this);

    SetSamplerStateImpl(filtering, repeating);
}

bool GpuTexture1D::IsTextureBound(eTextureUnit textureUnit) const
{
    if (!IsTextureInited())
        return false;

    debug_assert(textureUnit < eTextureUnit_COUNT);
    return this == mGraphicsContext.mCurrentTextures[textureUnit].mTexture1D;
}

bool GpuTexture1D::IsTextureBound() const
{
    if (!IsTextureInited())
        return false;

    for (int itexunit = 0; itexunit < eTextureUnit_COUNT; ++itexunit)
    {
        if (this == mGraphicsContext.mCurrentTextures[itexunit].mTexture1D)
            return true;
    }

    return false;
}

bool GpuTexture1D::Upload(const void* sourceData)
{
    if (!IsTextureInited())
        return false;

    debug_assert(sourceData);

    GLuint formatGL = 0;
    GLint internalFormatGL = 0;
    switch (mFormat)
    {
        case eTextureFormat_R8: 
            formatGL = GL_RED;
            internalFormatGL = GL_R8;
        break;
        case eTextureFormat_R8_G8: 
            formatGL = GL_RG;
            internalFormatGL = GL_RG8;
        break;
        case eTextureFormat_RGB8: 
            formatGL = GL_RGB;
            internalFormatGL = GL_RGB8;
        break;
        case eTextureFormat_RGBA8: 
            formatGL = GL_RGBA;
            internalFormatGL = GL_RGBA8;
        break;
    }
    ScopedTexture1DBinder scopedBind(mGraphicsContext, this);
    ::glTexImage1D(GL_TEXTURE_1D, 0, internalFormatGL, mSize.x, 0, formatGL, GL_UNSIGNED_BYTE, sourceData);
    glCheckError();
    return true;
}

bool GpuTexture1D::IsTextureInited() const
{
    return mFormat != eTextureFormat_Null;
}

void GpuTexture1D::SetSamplerStateImpl(eTextureFilterMode filtering, eTextureWrapMode repeating)
{
    debug_assert(filtering == eTextureFilterMode_Nearest);

    mFiltering = eTextureFilterMode_Nearest;
    mRepeating = repeating;

    // set filtering
    GLint magFilterGL = GL_NEAREST;
    GLint minFilterGL = GL_NEAREST;

    ::glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, magFilterGL);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, minFilterGL);
    glCheckError();

    // set repeating
    GLint wrapSGL = GL_CLAMP_TO_EDGE;
    GLint wrapTGL = GL_CLAMP_TO_EDGE;
    switch (repeating)
    {
        case eTextureWrapMode_Repeat:
            wrapSGL = GL_REPEAT;
            wrapTGL = GL_REPEAT;
        break;
        case eTextureWrapMode_ClampToEdge:
        break;
        default:
        {
            debug_assert(repeating == eTextureWrapMode_ClampToEdge);
        }
        break;
    }

    ::glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, wrapSGL);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, wrapTGL);
    glCheckError();
}

void GpuTexture1D::SetUnbound()
{
    for (int iTextureUnit = 0; iTextureUnit < eTextureUnit_COUNT; ++iTextureUnit)
    {
        if (this == mGraphicsContext.mCurrentTextures[iTextureUnit].mTexture1D)
        {
            mGraphicsContext.mCurrentTextures[iTextureUnit].mTexture1D = nullptr;
        }
    }
}
