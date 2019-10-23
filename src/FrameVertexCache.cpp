#include "stdafx.h"
#include "FrameVertexCache.h"
#include "GpuBuffer.h"

void TransientBuffer::SetSourceBuffer(GpuBuffer* bufferObject, unsigned int dataOffset, unsigned int dataLength)
{
    mGraphicsBuffer = bufferObject;
    mBufferDataOffset = dataOffset;
    mBufferDataLength = dataLength;
}

void TransientBuffer::SetNull()
{
    mGraphicsBuffer = nullptr;
    mBufferDataOffset = 0;
    mBufferDataLength = 0;
}

bool TransientBuffer::IsNull() const
{
    return mGraphicsBuffer == nullptr;
}

bool TransientBuffer::NonNull() const
{
    return mGraphicsBuffer != nullptr;
}

//////////////////////////////////////////////////////////////////////////

bool FrameVertexCache::Initialize()
{
    if (!InitFrameCacheBuffer(mFrameCache.mVertexCacheBuffer, eBufferContent_Vertices, MaxVertexBufferSize) ||
        !InitFrameCacheBuffer(mFrameCache.mIndexCacheBuffer, eBufferContent_Indices, MaxIndexBufferSize))
    {
        Deinit();
        return false;
    }
    return true;
}

void FrameVertexCache::Deinit()
{
    DeinitFrameCacheBuffer(mFrameCache.mVertexCacheBuffer);
    DeinitFrameCacheBuffer(mFrameCache.mIndexCacheBuffer);
}

void FrameVertexCache::FlushCache()
{
    SetFreeBuffers(mFrameCache.mVertexCacheBuffer);
    SetFreeBuffers(mFrameCache.mIndexCacheBuffer);
}

bool FrameVertexCache::InitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer, eBufferContent content, unsigned int maxBufferLength)
{
    // create initial buffer object

    cacheBuffer.mGraphicsBuffer = gGraphicsDevice.CreateBuffer(content, eBufferUsage_Stream, maxBufferLength, nullptr);
    if (cacheBuffer.mGraphicsBuffer)
    {
        cacheBuffer.mFreeLength = maxBufferLength;
        cacheBuffer.mUsedLength = 0;
        return true;
    }
    debug_assert(cacheBuffer.mGraphicsBuffer);
    return false;
}

void FrameVertexCache::DeinitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer)
{
    // destroy all allocated buffers

    if (cacheBuffer.mGraphicsBuffer)
    {
        gGraphicsDevice.DestroyBuffer(cacheBuffer.mGraphicsBuffer);
        cacheBuffer.mGraphicsBuffer = nullptr;
        cacheBuffer.mFreeLength = 0;
        cacheBuffer.mUsedLength = 0;
    }

    for (GpuBuffer* curr: cacheBuffer.mFullBuffers)
    {
        gGraphicsDevice.DestroyBuffer(curr);
    }

    for (GpuBuffer* curr: cacheBuffer.mFreeBuffers)
    {
        gGraphicsDevice.DestroyBuffer(curr);
    }

    cacheBuffer.mFullBuffers.clear();
    cacheBuffer.mFreeBuffers.clear();
}

void FrameVertexCache::SetFreeBuffers(FrameCacheBuffer& cacheBuffer)
{
    if (!cacheBuffer.mFullBuffers.empty())
    {
        cacheBuffer.mFreeBuffers.insert(cacheBuffer.mFreeBuffers.end(), 
            cacheBuffer.mFullBuffers.begin(), 
            cacheBuffer.mFullBuffers.end());
        cacheBuffer.mFullBuffers.clear();
    }
}

bool FrameVertexCache::AllocVertex(unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer)
{
    outputBuffer.SetNull();

    FrameCacheBuffer& cacheBuffer = mFrameCache.mVertexCacheBuffer;
    if (cacheBuffer.mGraphicsBuffer)
    {
        return TryAllocateData(cacheBuffer, dataLength, sourceData, outputBuffer);
    }
    debug_assert(false);
    return false;
}

bool FrameVertexCache::AllocIndex(unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer)
{
    outputBuffer.SetNull();

    FrameCacheBuffer& cacheBuffer = mFrameCache.mIndexCacheBuffer;
    if (cacheBuffer.mGraphicsBuffer)
    {
        return TryAllocateData(cacheBuffer, dataLength, sourceData, outputBuffer);
    }
    debug_assert(false);
    return false;
}

bool FrameVertexCache::RequestNextBuffer(FrameCacheBuffer& cacheBuffer)
{
    GpuBuffer* prevBufferObject = cacheBuffer.mGraphicsBuffer;
    GpuBuffer* nextBufferObject = nullptr;
    if (cacheBuffer.mFreeBuffers.size())
    {
        nextBufferObject = cacheBuffer.mFreeBuffers[0];
        cacheBuffer.mFreeBuffers.pop_front();

        nextBufferObject->Invalidate(); // do orphan trick
    }
    else // allocate new buffer
    {
        nextBufferObject = gGraphicsDevice.CreateBuffer(prevBufferObject->mContent, eBufferUsage_Stream, prevBufferObject->mBufferLength, nullptr);
    }
    debug_assert(nextBufferObject);
    if (nextBufferObject)
    {
        cacheBuffer.mFullBuffers.push_back(prevBufferObject);
        cacheBuffer.mGraphicsBuffer = nextBufferObject;
        cacheBuffer.mUsedLength = 0;
        cacheBuffer.mFreeLength = nextBufferObject->mBufferLength;
        return true;
    }

    return false;
}

bool FrameVertexCache::TryAllocateData(FrameCacheBuffer& cacheBuffer, unsigned long dataLength, void* sourceData, TransientBuffer& outputBuffer)
{
    debug_assert (cacheBuffer.mGraphicsBuffer);

    if (dataLength == 0 || dataLength > cacheBuffer.mGraphicsBuffer->mBufferLength)
    {
        debug_assert(false);
        return false;
    }

    // is there no free space left in current buffer?
    if (dataLength > cacheBuffer.mFreeLength)
    {
        if (!RequestNextBuffer(cacheBuffer))
            return false;
    }

    outputBuffer.SetSourceBuffer(cacheBuffer.mGraphicsBuffer, cacheBuffer.mUsedLength, dataLength);

    cacheBuffer.mUsedLength += dataLength;
    cacheBuffer.mFreeLength -= dataLength;

    // upload data right away
    if (sourceData)
    {
        if (void* destPointer = outputBuffer.mGraphicsBuffer->Lock(BufferAccess_UnsynchronizedWrite, 
            outputBuffer.mBufferDataOffset, outputBuffer.mBufferDataLength))
        {
            memcpy(destPointer, sourceData, dataLength);
            bool isSuccess = outputBuffer.mGraphicsBuffer->Unlock();
            debug_assert(isSuccess);
        }
    }
    return true;
}
