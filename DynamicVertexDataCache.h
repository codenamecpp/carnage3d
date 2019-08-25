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
    GpuBuffer* mBufferObject = nullptr; // temporary reference, it must not be stored
};

// defines cache system for dynamic geometry that being created each render frame
class DynamicVertexDataCache final: public cxx::noncopyable
{
public:
    bool Initialize();
    void Deinit();
    void RenderFrameEnd();
    // try to allocate vertex memory of specified size and upload source data
    // @param dataLength: Source data length in bytes
    // @param sourceData: Source data to upload
    // @param outputBuffer: Output transient buffer, only valid for one frame
    bool AllocateVertices(unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer);
    bool AllocateIndices(unsigned int dataLength, void* sourceData, TransientBuffer& outputBuffer);

private:
    static const unsigned int StartVertexBufferSize = (1024 * 1024) * 1; // 1 MB
    static const unsigned int StartIndexBufferSize = (1024 * 1024) * 1; // 1 MB
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