#pragma once

// defines part of dynamic vertex buffer
// the data within it is only valid for one frame of rendering
class TransientBuffer final
{
public:
    TransientBuffer() = default;
    // @param bufferObject: Source dynamic buffer
    // @param dataOffset, dataLength: Allocated area within buffer
    void SetSourceBuffer(GpuBuffer* bufferObject, unsigned int dataOffset, unsigned int dataLength);
    void SetNull();
    bool NonNull() const;
    bool IsNull() const;

public:
    // area of allocated data within buffer
    unsigned int mBufferDataOffset = 0;
    unsigned int mBufferDataLength = 0;

    GpuBuffer* mGraphicsBuffer = nullptr; // temporary reference, it must not be stored
};

// defines cache system for dynamic geometry that being created each render frame
class StreamingVertexCache final: public cxx::noncopyable
{
public:
    bool Initialize();
    void Deinit();
    void FlushCache();
    // try to allocate vertex memory of specified size and upload source data
    // @param dataLength: Source data length in bytes
    // @param sourceData: Source data to upload
    // @param outputBuffer: Output transient buffer, only valid for one frame
    bool AllocVertex(unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer);
    bool AllocIndex(unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer);

private:
    static const unsigned int MaxVertexBufferSize = (1024 * 1024) * 1; // 1 MB
    static const unsigned int MaxIndexBufferSize = (1024 * 1024) * 1; // 1 MB

    struct FrameCacheBuffer
    {
    public:
        GpuBuffer* mGraphicsBuffer = nullptr; // current buffer object to fill with data
        unsigned int mFreeLength = 0;
        unsigned int mUsedLength = 0;
        std::deque<GpuBuffer*> mFreeBuffers;
        std::deque<GpuBuffer*> mFullBuffers;
    };

    struct FrameCache
    {
    public:
        FrameCacheBuffer mVertexCacheBuffer;
        FrameCacheBuffer mIndexCacheBuffer;
    };

    bool InitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer, eBufferContent content, unsigned int maxBufferLength);
    void DeinitFrameCacheBuffer(FrameCacheBuffer& cacheBuffer);
    void SetFreeBuffers(FrameCacheBuffer& cacheBuffer);
    bool TryAllocateData(FrameCacheBuffer& cacheBuffer, unsigned long dataLength, void* sourceData, TransientBuffer& outputBuffer);
    bool RequestNextBuffer(FrameCacheBuffer& cacheBuffer);
    
    FrameCache mFrameCache;
};