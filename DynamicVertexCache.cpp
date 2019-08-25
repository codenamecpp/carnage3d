#include "stdafx.h"
#include "DynamicVertexCache.h"
#include "GpuBuffer.h"

TransientBuffer::~TransientBuffer()
{
    debug_assert(mDataPointer == nullptr);
}

void TransientBuffer::SetSourceBuffer(GpuBuffer* bufferObject, unsigned int dataOffset, unsigned int dataLength)
{
    mBufferObject = bufferObject;
    mBufferDataOffset = dataOffset;
    mBufferDataLength = dataLength;
    mDataPointer = nullptr;
}

void TransientBuffer::SetNull()
{
    mBufferObject = nullptr;
    mBufferDataOffset = 0;
    mBufferDataLength = 0;
    mDataPointer = nullptr;
}

void* TransientBuffer::Lock()
{
    debug_assert(mBufferObject);
    if (mDataPointer == nullptr && mBufferObject)
    {
        mDataPointer = mBufferObject->Lock(BufferAccess_UnsynchronizedWrite | BufferAccess_InvalidateRange, mBufferDataOffset, mBufferDataLength);
        debug_assert(mDataPointer);
    }
    return mDataPointer;
}

bool TransientBuffer::Unlock()
{
    if (mDataPointer && mBufferObject)
    {
        mDataPointer = nullptr;
        return mBufferObject->Unlock();
    }
    debug_assert(false);
    return false;
}

bool TransientBuffer::IsLocked() const
{
    return mDataPointer != nullptr;
}

bool TransientBuffer::IsNull() const
{
    return mBufferObject == nullptr;
}

bool TransientBuffer::NonNull() const
{
    return mBufferObject != nullptr;
}

//////////////////////////////////////////////////////////////////////////

bool DynamicVertexCache::Initialize()
{
    if (!InitFrameCacheBuffer(mFrameCache.mVertexCacheBuffer, eBufferContent_Vertices, MaxVertexBufferLength) ||
        !InitFrameCacheBuffer(mFrameCache.mIndexCacheBuffer, eBufferContent_Indices, MaxIndexBufferLength))
    {
        Deinit();
        return false;
    }
    return true;
}

void DynamicVertexCache::Deinit()
{
    DeinitFrameCacheBuffer(mFrameCache.mVertexCacheBuffer);
    DeinitFrameCacheBuffer(mFrameCache.mIndexCacheBuffer);
}

void DynamicVertexCache::RenderFrameBegin()
{
    SetCurrentFrameOffset(mFrameCache.mVertexCacheBuffer);
    SetCurrentFrameOffset(mFrameCache.mIndexCacheBuffer);
}

void DynamicVertexCache::RenderFrameEnd()
{
}

bool DynamicVertexCache::InitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer, eBufferContent content, unsigned long bufferLength)
{
    cacheBuffer.mBufferObject = gGraphicsDevice.CreateBuffer(content, eBufferUsage_Stream, bufferLength, nullptr);
    debug_assert(cacheBuffer.mBufferObject);
    cacheBuffer.mCurrentOffset = 0;
    cacheBuffer.mFrameStartOffset = 0;
    return cacheBuffer.mBufferObject != nullptr;
}

void DynamicVertexCache::DeinitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer)
{
    if (cacheBuffer.mBufferObject)
    {
        gGraphicsDevice.DestroyBuffer(cacheBuffer.mBufferObject);
        cacheBuffer.mBufferObject = nullptr;
        cacheBuffer.mCurrentOffset = 0;
        cacheBuffer.mFrameStartOffset = 0;
    }
}

void DynamicVertexCache::SetCurrentFrameOffset(FrameCacheBuffer& cacheBuffer)
{
    cacheBuffer.mFrameStartOffset = cacheBuffer.mCurrentOffset;
}

bool DynamicVertexCache::TryAllocateData(FrameCacheBuffer& cacheBuffer, unsigned long dataLength, void* sourceData, TransientBuffer& outputBuffer)
{
    debug_assert (cacheBuffer.mBufferObject);

    unsigned int maxLength = cacheBuffer.mBufferObject->mBufferCapacity;
    if (maxLength < dataLength || dataLength == 0)
    {
        debug_assert(false);
        return false;
    }

    int dataStartOffset = cacheBuffer.mCurrentOffset;

    // not enough space left, try from start
    if (dataStartOffset + dataLength > maxLength)
    {
        // overlaps current frame data, out of memory
        if (dataLength > cacheBuffer.mFrameStartOffset)
        {
            debug_assert(false);
            return false;
        }
        dataStartOffset = 0;
    }

    cacheBuffer.mCurrentOffset = dataStartOffset + dataLength;
    debug_assert(cacheBuffer.mCurrentOffset <= maxLength);
    outputBuffer.SetSourceBuffer(cacheBuffer.mBufferObject, dataStartOffset, dataLength);

    // upload data right away
    if (sourceData)
    {
        if (void* destPointer = outputBuffer.Lock())
        {
            memcpy(destPointer, sourceData, dataLength);
            bool isSuccess = outputBuffer.Unlock();
            debug_assert(isSuccess);
        }
    }
    return true;
}

bool DynamicVertexCache::Allocate(eBufferContent content, unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer)
{
    FrameCacheBuffer& cacheBuffer = (content == eBufferContent_Vertices) ? mFrameCache.mVertexCacheBuffer : mFrameCache.mIndexCacheBuffer;

    outputBuffer.SetNull();
    if (cacheBuffer.mBufferObject)
    {
        return TryAllocateData(cacheBuffer, dataLength, sourceData, outputBuffer);
    }
    debug_assert(false);
    return false;
}
