#include "stdafx.h"
#include "GpuBuffer.h"
#include "GraphicsContext.h"
#include "OpenGLDefs.h"

//////////////////////////////////////////////////////////////////////////

class ScopedBufferBinder
{
public:
    ScopedBufferBinder(GraphicsContext& renderContext, GpuBuffer* gpuBuffer)
        : mPreviousBuffer(renderContext.mCurrentBuffers[gpuBuffer->mContent])
        , mBuffer(gpuBuffer)
        , mRenderContext(renderContext)
    {
        debug_assert(mBuffer);
        if (mBuffer != mPreviousBuffer)
        {
            GLenum bufferTargetGL = EnumToGL(mBuffer->mContent);

            ::glBindBuffer(bufferTargetGL, mBuffer->mResourceHandle);
            glCheckError();
        }
    }
    ~ScopedBufferBinder()
    {
        if (mBuffer != mPreviousBuffer)
        {
            GLenum bufferTargetGL = EnumToGL(mBuffer->mContent);

            ::glBindBuffer(bufferTargetGL, mPreviousBuffer ? mPreviousBuffer->mResourceHandle : 0);
            glCheckError();
        }
    }
private:
    GraphicsContext& mRenderContext;
    GpuBuffer* mPreviousBuffer;
    GpuBuffer* mBuffer;
};

//////////////////////////////////////////////////////////////////////////

GpuBuffer::GpuBuffer(GraphicsContext& graphicsContext)
    : mGraphicsContext(graphicsContext)
    , mResourceHandle()
    , mContent()
    , mUsageHint()
    , mBufferLength()
    , mBufferCapacity()
{
}

GpuBuffer::~GpuBuffer()
{
    Deinit();
}

bool GpuBuffer::Create(eBufferContent bufferContent, eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer)
{
    unsigned int paddedContentLength = (bufferLength + 15U) & (~15U);

    Deinit();

    mBufferLength = bufferLength;
    mBufferCapacity = paddedContentLength;

    debug_assert(mContent < eBufferContent_COUNT);
    mContent = bufferContent;

    mUsageHint = bufferUsage;
    debug_assert(mUsageHint < eBufferUsage_COUNT);

    ::glGenBuffers(1, &mResourceHandle);
    glCheckError();

    ScopedBufferBinder scopedBind (mGraphicsContext, this);
    GLenum bufferTargetGL = EnumToGL(mContent);
    GLenum bufferUsageGL = EnumToGL(mUsageHint);
    ::glBufferData(bufferTargetGL, mBufferCapacity, nullptr, bufferUsageGL);
    glCheckError();

    if (dataBuffer)
    {
        void* pMappedData = ::glMapBufferRange(bufferTargetGL, 0, mBufferCapacity, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        glCheckError();
        if (pMappedData == nullptr)
        {
            debug_assert(false);
        }
        else
        {
            ::memcpy(pMappedData, dataBuffer, bufferLength);
        }

        GLboolean unmapResult = ::glUnmapBuffer(bufferTargetGL);
        glCheckError();
        debug_assert(unmapResult == GL_TRUE);
    }
    return true;
}

void GpuBuffer::Deinit()
{
    if (!IsBufferInited())
        return;

    // set unbound
    if (this == mGraphicsContext.mCurrentBuffers[eBufferContent_Vertices])
    {
        mGraphicsContext.mCurrentBuffers[eBufferContent_Vertices] = nullptr;
    }

    if (this == mGraphicsContext.mCurrentBuffers[eBufferContent_Indices])
    {
        mGraphicsContext.mCurrentBuffers[eBufferContent_Indices] = nullptr;
    }
    ::glDeleteBuffers(1, &mResourceHandle);
    glCheckError();
}

void* GpuBuffer::Lock(BufferAccessBits accessBits)
{
    return Lock(accessBits, 0, mBufferCapacity);
}

void* GpuBuffer::Lock(BufferAccessBits accessBits, unsigned int bufferOffset, unsigned int dataLength)
{
    if (!IsBufferInited())
    {
        debug_assert(false);
        return nullptr;
    }

    if (mBufferCapacity == 0)
    {
        debug_assert(false);
        return false;
    }

    GLbitfield accessBitsGL = ((accessBits & BufferAccess_Read) > 0 ? GL_MAP_READ_BIT : 0) |
        ((accessBits & BufferAccess_Write) > 0 ? GL_MAP_WRITE_BIT : 0) |
        ((accessBits & BufferAccess_Unsynchronized) > 0 ? GL_MAP_UNSYNCHRONIZED_BIT : 0) |
        ((accessBits & BufferAccess_InvalidateRange) > 0 ? GL_MAP_INVALIDATE_RANGE_BIT : 0) |
        ((accessBits & BufferAccess_InvalidateBuffer) > 0 ? GL_MAP_INVALIDATE_BUFFER_BIT : 0);

    debug_assert(accessBitsGL > 0);
    if (accessBitsGL == 0)
        return nullptr;

    if (dataLength == 0 || bufferOffset + dataLength > mBufferCapacity)
    {
        debug_assert(false);
        return nullptr;
    }

    ScopedBufferBinder scopedBind (mGraphicsContext, this);
    GLenum bufferTargetGL = EnumToGL(mContent);
    void* pMappedData = ::glMapBufferRange(bufferTargetGL, bufferOffset, dataLength, accessBitsGL);
    glCheckError();
    return pMappedData;
}

bool GpuBuffer::Unlock()
{
    if (!IsBufferInited())
    {
        debug_assert(false);
        return false;
    }

    ScopedBufferBinder scopedBind (mGraphicsContext, this);
    GLenum bufferTargetGL = EnumToGL(mContent);
    GLboolean unmapResult = ::glUnmapBuffer(bufferTargetGL);
    glCheckError();
    return unmapResult == GL_TRUE;
}

void GpuBuffer::Invalidate()
{
    if (!IsBufferInited())
    {
        debug_assert(false);
        return;
    }

    if (mBufferCapacity == 0)
    {
        debug_assert(false);
        return;
    }

    ScopedBufferBinder scopedBind (mGraphicsContext, this);
    GLenum bufferTargetGL = EnumToGL(mContent);
    GLenum bufferUsageGL = EnumToGL(mUsageHint);
    ::glBufferData(bufferTargetGL, mBufferCapacity, nullptr, bufferUsageGL);
    glCheckError();
}

bool GpuBuffer::IsBufferBound() const
{
    if (IsBufferInited())
    {
        return this == mGraphicsContext.mCurrentBuffers[mContent];
    }
    return false;
}

bool GpuBuffer::IsBufferInited() const
{
    return mResourceHandle > 0;
}
