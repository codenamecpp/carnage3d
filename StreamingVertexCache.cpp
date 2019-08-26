#include "stdafx.h"
#include "StreamingVertexCache.h"
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

bool StreamingVertexCache::Initialize()
{
    for (int iframe = 0; iframe < NumFrames; ++iframe)
    {
        if (!InitFrameCacheBuffer(mFrameCache[iframe].mVertexCacheBuffer, eBufferContent_Vertices, StartVertexBufferSize) ||
            !InitFrameCacheBuffer(mFrameCache[iframe].mIndexCacheBuffer, eBufferContent_Indices, StartIndexBufferSize))
        {
            Deinit();
            return false;
        }
    }
    mCurrentFrame = 0;
    return true;
}

void StreamingVertexCache::Deinit()
{
    for (int iframe = 0; iframe < NumFrames; ++iframe)
    {
        DeinitFrameCacheBuffer(mFrameCache[iframe].mVertexCacheBuffer);
        DeinitFrameCacheBuffer(mFrameCache[iframe].mIndexCacheBuffer);
    }
}

void StreamingVertexCache::RenderFrameEnd()
{
    for (int iframe = 0; iframe < NumFrames; ++iframe)
    {
        SetCurrentFrameOffset(mFrameCache[iframe].mVertexCacheBuffer);
        SetCurrentFrameOffset(mFrameCache[iframe].mIndexCacheBuffer);
    }
    mCurrentFrame = (mCurrentFrame + 1) % NumFrames;
}

bool StreamingVertexCache::InitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer, eBufferContent content, unsigned long bufferLength)
{
    cacheBuffer.mGraphicsBuffer = gGraphicsDevice.CreateBuffer(content, eBufferUsage_Stream, bufferLength, nullptr);
    debug_assert(cacheBuffer.mGraphicsBuffer);
    cacheBuffer.mCurrentOffset = 0;
    cacheBuffer.mFrameStartOffset = 0;
    return cacheBuffer.mGraphicsBuffer != nullptr;
}

void StreamingVertexCache::DeinitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer)
{
    if (cacheBuffer.mGraphicsBuffer)
    {
        gGraphicsDevice.DestroyBuffer(cacheBuffer.mGraphicsBuffer);
        cacheBuffer.mGraphicsBuffer = nullptr;
        cacheBuffer.mCurrentOffset = 0;
        cacheBuffer.mFrameStartOffset = 0;
    }
}

void StreamingVertexCache::SetCurrentFrameOffset(FrameCacheBuffer& cacheBuffer)
{
    cacheBuffer.mFrameStartOffset = cacheBuffer.mCurrentOffset;
}

bool StreamingVertexCache::TryAllocateData(FrameCacheBuffer& cacheBuffer, unsigned long dataLength, void* sourceData, TransientBuffer& outputBuffer)
{
    debug_assert (cacheBuffer.mGraphicsBuffer);

    unsigned int maxLength = cacheBuffer.mGraphicsBuffer->mBufferCapacity;
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
            // grow
            maxLength = static_cast<unsigned int>((maxLength + dataLength) * 1.6f);

            bool isSuccess = cacheBuffer.mGraphicsBuffer->Resize(maxLength);
            debug_assert(isSuccess);

            if (!isSuccess)
                return false;
        }
        else
        {
            dataStartOffset = 0;
        }
    }

    cacheBuffer.mCurrentOffset = dataStartOffset + dataLength;
    debug_assert(cacheBuffer.mCurrentOffset <= maxLength);
    outputBuffer.SetSourceBuffer(cacheBuffer.mGraphicsBuffer, dataStartOffset, dataLength);

    // upload data right away
    if (sourceData)
    {
        if (void* destPointer = outputBuffer.mGraphicsBuffer->Lock(BufferAccess_UnsynchronizedWrite | BufferAccess_InvalidateRange, 
            outputBuffer.mBufferDataOffset, outputBuffer.mBufferDataLength))
        {
            memcpy(destPointer, sourceData, dataLength);
            bool isSuccess = outputBuffer.mGraphicsBuffer->Unlock();
            debug_assert(isSuccess);
        }
    }
    return true;
}

bool StreamingVertexCache::AllocVertex(unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer)
{
    outputBuffer.SetNull();

    FrameCacheBuffer& cacheBuffer = mFrameCache[mCurrentFrame].mVertexCacheBuffer;
    if (cacheBuffer.mGraphicsBuffer)
    {
        return TryAllocateData(cacheBuffer, dataLength, sourceData, outputBuffer);
    }
    debug_assert(false);
    return false;
}

bool StreamingVertexCache::AllocIndex(unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer)
{
    outputBuffer.SetNull();

    FrameCacheBuffer& cacheBuffer = mFrameCache[mCurrentFrame].mIndexCacheBuffer;
    if (cacheBuffer.mGraphicsBuffer)
    {
        return TryAllocateData(cacheBuffer, dataLength, sourceData, outputBuffer);
    }
    debug_assert(false);
    return false;
}