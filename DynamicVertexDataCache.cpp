#include "stdafx.h"
#include "DynamicVertexDataCache.h"
#include "GpuBuffer.h"

void TransientBuffer::SetSourceBuffer(GpuBuffer* bufferObject, unsigned int dataOffset, unsigned int dataLength)
{
    mBufferObject = bufferObject;
    mBufferDataOffset = dataOffset;
    mBufferDataLength = dataLength;
}

void TransientBuffer::SetNull()
{
    mBufferObject = nullptr;
    mBufferDataOffset = 0;
    mBufferDataLength = 0;
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

bool DynamicVertexDataCache::Initialize()
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

void DynamicVertexDataCache::Deinit()
{
    for (int iframe = 0; iframe < NumFrames; ++iframe)
    {
        DeinitFrameCacheBuffer(mFrameCache[iframe].mVertexCacheBuffer);
        DeinitFrameCacheBuffer(mFrameCache[iframe].mIndexCacheBuffer);
    }
}

void DynamicVertexDataCache::RenderFrameEnd()
{
    for (int iframe = 0; iframe < NumFrames; ++iframe)
    {
        SetCurrentFrameOffset(mFrameCache[iframe].mVertexCacheBuffer);
        SetCurrentFrameOffset(mFrameCache[iframe].mIndexCacheBuffer);
    }
    mCurrentFrame = (mCurrentFrame + 1) % NumFrames;
}

bool DynamicVertexDataCache::InitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer, eBufferContent content, unsigned long bufferLength)
{
    cacheBuffer.mBufferObject = gGraphicsDevice.CreateBuffer(content, eBufferUsage_Stream, bufferLength, nullptr);
    debug_assert(cacheBuffer.mBufferObject);
    cacheBuffer.mCurrentOffset = 0;
    cacheBuffer.mFrameStartOffset = 0;
    return cacheBuffer.mBufferObject != nullptr;
}

void DynamicVertexDataCache::DeinitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer)
{
    if (cacheBuffer.mBufferObject)
    {
        gGraphicsDevice.DestroyBuffer(cacheBuffer.mBufferObject);
        cacheBuffer.mBufferObject = nullptr;
        cacheBuffer.mCurrentOffset = 0;
        cacheBuffer.mFrameStartOffset = 0;
    }
}

void DynamicVertexDataCache::SetCurrentFrameOffset(FrameCacheBuffer& cacheBuffer)
{
    cacheBuffer.mFrameStartOffset = cacheBuffer.mCurrentOffset;
}

bool DynamicVertexDataCache::TryAllocateData(FrameCacheBuffer& cacheBuffer, unsigned long dataLength, void* sourceData, TransientBuffer& outputBuffer)
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
            // grow
            maxLength = static_cast<unsigned int>((maxLength + dataLength) * 1.6f);

            bool isSuccess = cacheBuffer.mBufferObject->Resize(maxLength);
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
    outputBuffer.SetSourceBuffer(cacheBuffer.mBufferObject, dataStartOffset, dataLength);

    // upload data right away
    if (sourceData)
    {
        if (void* destPointer = outputBuffer.mBufferObject->Lock(BufferAccess_UnsynchronizedWrite | BufferAccess_InvalidateRange, 
            outputBuffer.mBufferDataOffset, outputBuffer.mBufferDataLength))
        {
            memcpy(destPointer, sourceData, dataLength);
            bool isSuccess = outputBuffer.mBufferObject->Unlock();
            debug_assert(isSuccess);
        }
    }
    return true;
}

bool DynamicVertexDataCache::AllocVertex(unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer)
{
    outputBuffer.SetNull();

    FrameCacheBuffer& cacheBuffer = mFrameCache[mCurrentFrame].mVertexCacheBuffer;
    if (cacheBuffer.mBufferObject)
    {
        return TryAllocateData(cacheBuffer, dataLength, sourceData, outputBuffer);
    }
    debug_assert(false);
    return false;
}

bool DynamicVertexDataCache::AllocIndex(unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer)
{
    outputBuffer.SetNull();

    FrameCacheBuffer& cacheBuffer = mFrameCache[mCurrentFrame].mIndexCacheBuffer;
    if (cacheBuffer.mBufferObject)
    {
        return TryAllocateData(cacheBuffer, dataLength, sourceData, outputBuffer);
    }
    debug_assert(false);
    return false;
}