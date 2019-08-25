#pragma once

// defines part of dynamic vertex buffer
// the data within it is only valid for one frame of rendering
class TransientBuffer final
{
public:
    TransientBuffer() = default;
    ~TransientBuffer();

    // @param bufferObject: Source dynamic buffer
    // @param dataOffset, dataLength: Allocated area within buffer
    void SetSourceBuffer(GpuBuffer* bufferObject, unsigned int dataOffset, unsigned int dataLength);
    void SetNull();
    bool NonNull() const;
    bool IsNull() const;
    void* Lock();
    bool Unlock();
    bool IsLocked() const;

public:
    void* mDataPointer = nullptr; // write only, it valid while buffer is locked
    // area of allocated data within buffer
    unsigned int mBufferDataOffset = 0;
    unsigned int mBufferDataLength = 0;
    GpuBuffer* mBufferObject = nullptr; // temporary reference, it must not be stored
};

// defines cache system for dynamic geometry that being created each render frame
class DynamicVertexCache final: public cxx::noncopyable
{
public:
    bool Initialize();
    void Deinit();
    void RenderFrameBegin();
    void RenderFrameEnd();
    // try to allocate vertex memory of specified size and upload source data
    // @param content: Type of source data content
    // @param dataLength: Source data length in bytes
    // @param sourceData: Source data, optional
    // @param outputBuffer: Output transient buffer, only valid for one frame
    bool Allocate(eBufferContent content, unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer);

private:
    static const unsigned int MaxVertexBufferLength = 2 * 1024 * 1024; // 2 MB
    static const unsigned int MaxIndexBufferLength = 2 * 1024 * 1024; // 2 MB
    static const int NumFrames = 2;

    struct FrameCacheBuffer
    {
    public:
        unsigned int mFrameStartOffset = 0;
        unsigned int mCurrentOffset = 0;

        GpuBuffer* mBufferObject = nullptr;
    };

    struct FrameCache
    {
    public:
        FrameCacheBuffer mVertexCacheBuffer;
        FrameCacheBuffer mIndexCacheBuffer;
    };

    bool InitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer, eBufferContent content, unsigned long bufferLength);
    void DeinitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer);
    void SetCurrentFrameOffset(FrameCacheBuffer& cacheBuffer);
    bool TryAllocateData(FrameCacheBuffer& cacheBuffer, unsigned long dataLength, void* sourceData, TransientBuffer& outputBuffer);

    int mCurrentFrame = 0;
    FrameCache mFrameCache[NumFrames];
};