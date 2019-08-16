#include "stdafx.h"
#include "GpuTexture2D.h"
#include "OpenGLDefs.h"
#include "GraphicsContext.h"

//////////////////////////////////////////////////////////////////////////

class ScopedTexture2DBinder
{
public:
    ScopedTexture2DBinder(GraphicsContext& renderContext, GpuTexture2D* gpuTexture)
        : mRenderContext(renderContext)
        , mPreviousTexture(renderContext.mCurrentTextures2D[renderContext.mCurrentTextureUnit])
        , mTexture(gpuTexture)
    {
        debug_assert(gpuTexture);
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_2D, mTexture->mResourceHandle);
            glCheckError();
        }
    }
    ~ScopedTexture2DBinder()
    {
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_2D, mPreviousTexture ? mPreviousTexture->mResourceHandle : 0);
            glCheckError();
        }
    }
private:
    GraphicsContext& mRenderContext;
    GpuTexture2D* mPreviousTexture;
    GpuTexture2D* mTexture;
};

//////////////////////////////////////////////////////////////////////////

GpuTexture2D::GpuTexture2D(GraphicsContext& renderContext)
    : mGraphicsContext(renderContext)
    , mResourceHandle()
    , mFiltering()
    , mRepeating()
    , mSize()
    , mFormat()
{
}

GpuTexture2D::~GpuTexture2D()
{
    Deinit();
}

void GpuTexture2D::Deinit()
{
    if (!IsTextureInited())
        return;

    // set unbound
    for (int iTextureUnit = 0; iTextureUnit < eTextureUnit_COUNT; ++iTextureUnit)
    {
        if (this == mGraphicsContext.mCurrentTextures2D[iTextureUnit])
        {
            mGraphicsContext.mCurrentTextures2D[iTextureUnit] = nullptr;
        }
    }
    ::glDeleteTextures(1, &mResourceHandle);
    glCheckError();
}

bool GpuTexture2D::Create(eTextureFormat textureFormat, int sizex, int sizey, const void* sourceData)
{
    Deinit();

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

    ::glGenTextures(1, &mResourceHandle);
    glCheckError();

    mFormat = textureFormat;
    mSize.x = sizex;
    mSize.y = sizey;
    
    ScopedTexture2DBinder scopedBind(mGraphicsContext, this);
    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mSize.x, mSize.y, 0, formatGL, GL_UNSIGNED_BYTE, sourceData);
    glCheckError();

    // set default filter and repeat mode for texture
    SetSamplerStateImpl(gGraphicsDevice.mDefaultTextureFilter, gGraphicsDevice.mDefaultTextureWrap);
    return true;
}

void GpuTexture2D::SetSamplerState(eTextureFilterMode filtering, eTextureWrapMode repeating)
{
    if (!IsTextureInited())
    {
        debug_assert(false);
        return;
    }

    // already set
    if (mFiltering == filtering && mRepeating == repeating)
        return;

    ScopedTexture2DBinder scopedBind(mGraphicsContext, this);

    SetSamplerStateImpl(filtering, repeating);
}

bool GpuTexture2D::IsTextureBound(eTextureUnit textureUnit) const
{
    if (!IsTextureInited())
        return false;

    debug_assert(textureUnit < eTextureUnit_COUNT);
    return this == mGraphicsContext.mCurrentTextures2D[textureUnit];
}

bool GpuTexture2D::IsTextureBound() const
{
    if (!IsTextureInited())
        return false;

    for (int itexunit = 0; itexunit < eTextureUnit_COUNT; ++itexunit)
    {
        if (this == mGraphicsContext.mCurrentTextures2D[itexunit])
            return true;
    }

    return false;
}

void GpuTexture2D::Invalidate()
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
    ScopedTexture2DBinder scopedBind(mGraphicsContext, this);
    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mSize.x, mSize.y, 0, formatGL, GL_UNSIGNED_BYTE, nullptr);
    glCheckError();
}

bool GpuTexture2D::Upload(const void* sourceData)
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
    ScopedTexture2DBinder scopedBind(mGraphicsContext, this);
    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mSize.x, mSize.y, 0, formatGL, GL_UNSIGNED_BYTE, sourceData);
    glCheckError();
    return true;
}

bool GpuTexture2D::IsTextureInited() const
{
    return mResourceHandle > 0;
}

void GpuTexture2D::SetSamplerStateImpl(eTextureFilterMode filtering, eTextureWrapMode repeating)
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
