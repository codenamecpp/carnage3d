#include "stdafx.h"
#include "GpuTextureArray2D.h"
#include "OpenGLDefs.h"
#include "GraphicsContext.h"

//////////////////////////////////////////////////////////////////////////

class ScopedTextureArray2DBinder
{
public:
    ScopedTextureArray2DBinder(GraphicsContext& renderContext, GpuTextureArray2D* gpuTexture)
        : mRenderContext(renderContext)
        , mPreviousTexture(renderContext.mCurrentTextures[renderContext.mCurrentTextureUnit].mTextureArray2D)
        , mTexture(gpuTexture)
    {
        debug_assert(gpuTexture);
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_2D_ARRAY, mTexture->mResourceHandle);
            glCheckError();
        }
    }
    ~ScopedTextureArray2DBinder()
    {
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_2D_ARRAY, mPreviousTexture ? mPreviousTexture->mResourceHandle : 0);
            glCheckError();
        }
    }
private:
    GraphicsContext& mRenderContext;
    GpuTextureArray2D* mPreviousTexture;
    GpuTextureArray2D* mTexture;
};

//////////////////////////////////////////////////////////////////////////

GpuTextureArray2D::GpuTextureArray2D(GraphicsContext& graphicsContext)
    : mGraphicsContext(graphicsContext)
    , mResourceHandle()
    , mFiltering()
    , mRepeating()
    , mSize()
    , mFormat()
    , mLayersCount()
{
    ::glGenTextures(1, &mResourceHandle);
    glCheckError();
}

GpuTextureArray2D::~GpuTextureArray2D()
{
    SetUnbound();

    ::glDeleteTextures(1, &mResourceHandle);
    glCheckError();
}

bool GpuTextureArray2D::Setup(eTextureFormat textureFormat, int sizex, int sizey, int layersCount, const void* sourceData)
{
    GLuint formatGL = GetTextureInputFormatGL(textureFormat);
    GLint internalFormatGL = GetTextureInternalFormatGL(textureFormat);
    GLenum dataType = GetTextureDataTypeGL(textureFormat);
    if (formatGL == 0 || internalFormatGL == 0 || dataType == 0)
    {
        debug_assert(false);
        return false;
    }

    mFormat = textureFormat;
    mSize.x = sizex;
    mSize.y = sizey;
    mLayersCount = layersCount;

    int MaxLayers = gGraphicsDevice.mCaps.mMaxArrayTextureLayers;
    debug_assert(MaxLayers >= mLayersCount);
    if (MaxLayers < mLayersCount)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Exceeded number of texture array layers (%d, max is %d)", mLayersCount, MaxLayers);
        mLayersCount = MaxLayers;
    }
    
    ScopedTextureArray2DBinder scopedBind(mGraphicsContext, this);

    ::glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalFormatGL, mSize.x, mSize.y, layersCount);
    glCheckError();

    if (sourceData)
    {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, mSize.x, mSize.y, layersCount, formatGL, dataType, sourceData);
        glCheckError();
    }

    // set default filter and repeat mode for texture
    SetSamplerStateImpl(gGraphicsDevice.mDefaultTextureFilter, gGraphicsDevice.mDefaultTextureWrap);
    return true;
}

bool GpuTextureArray2D::Upload(int startLayerIndex, int layersCount, const void* sourceData)
{
    if (!IsTextureInited())
        return false;

    debug_assert(sourceData);
    debug_assert(mLayersCount >= (startLayerIndex + layersCount));

    GLuint formatGL = GetTextureInputFormatGL(mFormat);
    GLint internalFormatGL = GetTextureInternalFormatGL(mFormat);
    GLenum dataType = GetTextureDataTypeGL(mFormat);
    if (formatGL == 0 || internalFormatGL == 0 || dataType == 0)
    {
        debug_assert(false);
        return false;
    }

    ScopedTextureArray2DBinder scopedBind(mGraphicsContext, this);

    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, startLayerIndex, mSize.x, mSize.y, layersCount, formatGL, dataType, sourceData);
    glCheckError();
    return true;
}

void GpuTextureArray2D::SetSamplerState(eTextureFilterMode filtering, eTextureWrapMode repeating)
{
    if (!IsTextureInited())
    {
        debug_assert(false);
        return;
    }

    // already set
    if (mFiltering == filtering && mRepeating == repeating)
        return;

    ScopedTextureArray2DBinder scopedBind(mGraphicsContext, this);

    SetSamplerStateImpl(filtering, repeating);
}

bool GpuTextureArray2D::IsTextureBound(eTextureUnit textureUnit) const
{
    if (!IsTextureInited())
        return false;

    debug_assert(textureUnit < eTextureUnit_COUNT);
    return this == mGraphicsContext.mCurrentTextures[textureUnit].mTextureArray2D;
}

bool GpuTextureArray2D::IsTextureBound() const
{
    if (!IsTextureInited())
        return false;

    for (int itexunit = 0; itexunit < eTextureUnit_COUNT; ++itexunit)
    {
        if (this == mGraphicsContext.mCurrentTextures[itexunit].mTextureArray2D)
            return true;
    }

    return false;
}

bool GpuTextureArray2D::IsTextureInited() const
{
    return mFormat != eTextureFormat_Null;
}

void GpuTextureArray2D::SetSamplerStateImpl(eTextureFilterMode filtering, eTextureWrapMode repeating)
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

    ::glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, magFilterGL);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilterGL);
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

    ::glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapSGL);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapTGL);
    glCheckError();
}

void GpuTextureArray2D::SetUnbound()
{
    for (int iTextureUnit = 0; iTextureUnit < eTextureUnit_COUNT; ++iTextureUnit)
    {
        if (this == mGraphicsContext.mCurrentTextures[iTextureUnit].mTextureArray2D)
        {
            mGraphicsContext.mCurrentTextures[iTextureUnit].mTextureArray2D = nullptr;
        }
    }
}