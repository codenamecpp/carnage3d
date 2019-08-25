#pragma once

#include "GraphicsDefs.h"

// defines hardware vertices/indices buffer object
class GpuBuffer final: public cxx::noncopyable
{
public:
    // public for convenience, don't change these fields directly
    GpuBufferHandle mResourceHandle;
    eBufferContent mContent;
    eBufferUsage mUsageHint;
    unsigned int mBufferLength; // user requested length in bytes
    unsigned int mBufferCapacity; // actually allocated length int bytes

public:
    GpuBuffer(GraphicsContext& graphicsContext);
    ~GpuBuffer();

    // Will drop buffer data and allocate new chunk of gpu memory
    // @param bufferContent: Content type stored in buffer
    // @param bufferUsage: Usage hint of buffer
    // @param theLength: Data length
    // @param dataBuffer: Initial data, optional
    // @returns false if out of memory
    bool Setup(eBufferContent bufferContent, eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer);

    // Upload source data to buffer replacing old content
    // @param dataOffset: Offset within buffer to write in bytes
    // @param dataLength: Size of data to write in bytes
    // @param dataSource: Source data
    bool SubData(unsigned int dataOffset, unsigned int dataLength, const void* dataSource);

    // Map hardware buffer content to process memory
    // @param accessBits: Desired data access policy
    // @param bufferOffset: Offset from start, bytes
    // @param dataSize: Size of data, bytes
    // @return Pointer to buffer data or null on fail
    void* Lock(BufferAccessBits accessBits);
    void* Lock(BufferAccessBits accessBits, unsigned int bufferOffset, unsigned int dataSize);

    template<typename TElement>
    inline TElement* LockData(BufferAccessBits accessBits)
    {
        return static_cast<TElement*>(Lock(accessBits));
    }
    template<typename TElement>
    inline TElement* LockData(BufferAccessBits accessBits, unsigned int bufferOffset, unsigned int dataSize)
    {
        return static_cast<TElement*>(Lock(accessBits, bufferOffset, dataSize));
    }

    // Unmap buffer object data source
    // @return false on fail, indicates that buffer should be reload
    bool Unlock();

    // Will drop buffer data and allocate new chunk of gpu memory of same size
    // This is used in 'orphaning' technique 
    void Invalidate();

    // Test whether index or vertex attributes data is currently bound - depends on content
    bool IsBufferBound() const;

    // Test whether buffer is created
    bool IsBufferInited() const;

private:
    GraphicsContext& mGraphicsContext;
};