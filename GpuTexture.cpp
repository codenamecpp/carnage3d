#include "stdafx.h"
#include "GpuTexture.h"
#include "OpenGLDefs.h"
#include "GraphicsContext.h"

//////////////////////////////////////////////////////////////////////////

class ScopedTextureBinder
{
public:
    ScopedTextureBinder(GraphicsContext& renderContext, GpuTexture* gpuTexture)
        : mRenderContext(renderContext)
        , mPreviousTexture(renderContext.mCurrentTextures[renderContext.mCurrentTextureUnit])
        , mTexture(gpuTexture)
    {
        debug_assert(gpuTexture);
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_2D, mTexture->mResourceHandle);
            glCheckError();
        }
    }
    ~ScopedTextureBinder()
    {
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_2D, mPreviousTexture ? mPreviousTexture->mResourceHandle : 0);
            glCheckError();
        }
    }
private:
    GraphicsContext& mRenderContext;
    GpuTexture* mPreviousTexture;
    GpuTexture* mTexture;
};

//////////////////////////////////////////////////////////////////////////

GpuTexture::GpuTexture(GraphicsContext& renderContext)
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

GpuTexture::~GpuTexture()
{
    SetUnbound();

    ::glDeleteTextures(1, &mResourceHandle);
    glCheckError();
}

bool GpuTexture::Setup(eTextureFormat textureFormat, int sizex, int sizey, const void* sourceData)
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
    mSize.y = sizey;
    
    ScopedTextureBinder scopedBind(mGraphicsContext, this);
    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mSize.x, mSize.y, 0, formatGL, GL_UNSIGNED_BYTE, sourceData);
    glCheckError();

    // set default filter and repeat mode for texture
    SetSamplerStateImpl(gGraphicsDevice.mDefaultTextureFilter, gGraphicsDevice.mDefaultTextureWrap);
    return true;
}

void GpuTexture::SetSamplerState(eTextureFilterMode filtering, eTextureWrapMode repeating)
{
    if (!IsTextureInited())
    {
        debug_assert(false);
        return;
    }

    // already set
    if (mFiltering == filtering && mRepeating == repeating)
        return;

    ScopedTextureBinder scopedBind(mGraphicsContext, this);

    SetSamplerStateImpl(filtering, repeating);
}

bool GpuTexture::IsTextureBound(eTextureUnit textureUnit) const
{
    if (!IsTextureInited())
        return false;

    debug_assert(textureUnit < eTextureUnit_COUNT);
    return this == mGraphicsContext.mCurrentTextures[textureUnit];
}

bool GpuTexture::IsTextureBound() const
{
    if (!IsTextureInited())
        return false;

    for (int itexunit = 0; itexunit < eTextureUnit_COUNT; ++itexunit)
    {
        if (this == mGraphicsContext.mCurrentTextures[itexunit])
            return true;
    }

    return false;
}

void GpuTexture::Invalidate()
{
    if (!IsTextureInited())
        return;

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
    ScopedTextureBinder scopedBind(mGraphicsContext, this);
    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mSize.x, mSize.y, 0, formatGL, GL_UNSIGNED_BYTE, nullptr);
    glCheckError();
}

bool GpuTexture::Upload(const void* sourceData)
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
    ScopedTextureBinder scopedBind(mGraphicsContext, this);
    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mSize.x, mSize.y, 0, formatGL, GL_UNSIGNED_BYTE, sourceData);
    glCheckError();
    return true;
}

bool GpuTexture::IsTextureInited() const
{
    return mFormat != eTextureFormat_Null;
}

void GpuTexture::SetSamplerStateImpl(eTextureFilterMode filtering, eTextureWrapMode repeating)
{
    mFiltering = filtering;
    mRepeating = repeating;

    // set filtering
    GLint magFilterGL = GL_NEAREST;
    GLint minFilterGL = GL_NEAREST;
    switch (filtering)
    {
        case eTextureFilterMode_Nearest: 
        break;
        case eTextureFilterMode_Bilinear:
            magFilterGL = GL_LINEAR;
            minFilterGL = GL_LINEAR;
        break;
        case eTextureFilterMode_Trilinear:
            magFilterGL = GL_LINEAR;
            minFilterGL = GL_LINEAR;
        break;
        default:
        {
            debug_assert(filtering == eTextureFilterMode_Nearest);
        }
        break;
    }

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterGL);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterGL);
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

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSGL);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTGL);
    glCheckError();
}

void GpuTexture::SetUnbound()
{
    for (int iTextureUnit = 0; iTextureUnit < eTextureUnit_COUNT; ++iTextureUnit)
    {
        if (this == mGraphicsContext.mCurrentTextures[iTextureUnit])
        {
            mGraphicsContext.mCurrentTextures[iTextureUnit] = nullptr;
        }
    }
}
