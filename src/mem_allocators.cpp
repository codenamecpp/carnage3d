#include "stdafx.h"
#include "mem_allocators.h"

namespace cxx
{

struct linear_alloc_header
{
    unsigned int mAllocationLength; // header size not included
};

linear_memory_allocator::~linear_memory_allocator()
{
    if (mMemoryBuffer)
    {
        free(mMemoryBuffer);
        mMemoryBuffer = nullptr;
    }
}

bool linear_memory_allocator::init_allocator(unsigned int bufferSizeTotal)
{
    if (mMemoryBuffer)
    {
        free(mMemoryBuffer);
    }

    mMemoryBuffer = (unsigned char*) malloc(bufferSizeTotal);
    if (mMemoryBuffer == nullptr)
    {
        return false;
    }

    mMemorySizeTotal = bufferSizeTotal;
    mMemorySizeUsed = 0;
    mMemorySizeFree = bufferSizeTotal;
    return true;
}

void* linear_memory_allocator::allocate(unsigned int dataLength)
{
    unsigned int allocPos = cxx::align_up(mMemorySizeUsed, 16);
    if (allocPos + dataLength + sizeof(linear_alloc_header) <= mMemorySizeFree)
    {
        unsigned char* dataPointer = ((unsigned char*) mMemoryBuffer) + allocPos;

        // write header
        linear_alloc_header* headerPointer = (linear_alloc_header*) dataPointer;
        headerPointer->mAllocationLength = dataLength;

        mMemorySizeUsed = allocPos + sizeof(linear_alloc_header) + dataLength;
        mMemorySizeFree = mMemorySizeTotal - mMemorySizeUsed;
        return dataPointer + sizeof(linear_alloc_header);
    }
    else
    {   
        // report overflow
        if (mOutOfMemoryProc)
        {
            mOutOfMemoryProc(dataLength);
        }
    }
    return nullptr;
}

void* linear_memory_allocator::reallocate(void* dataPointer, unsigned int dataLength)
{
    unsigned char* sourcePointer = (unsigned char*) dataPointer;

    // sanity check
    debug_assert(sourcePointer >= mMemoryBuffer && sourcePointer <= (mMemoryBuffer + mMemorySizeTotal));

    // get previous allocation header
    linear_alloc_header* headerPointer = (linear_alloc_header*) (sourcePointer - sizeof(linear_alloc_header));
    // allocate new chunk
    dataPointer = allocate(dataLength);
    if (dataPointer) // copy old memory
    {
        memcpy(dataPointer, sourcePointer, headerPointer->mAllocationLength);
        return dataPointer;
    }
    return nullptr;
}

void linear_memory_allocator::deallocate(void* dataPointer)
{
    unsigned char* sourcePointer = (unsigned char*) dataPointer;
    // sanity check
    debug_assert(sourcePointer >= mMemoryBuffer && sourcePointer <= (mMemoryBuffer + mMemorySizeTotal));
    
    // can only free very last allocation
    linear_alloc_header* headerPointer = (linear_alloc_header*) (sourcePointer - sizeof(linear_alloc_header));
    if (sourcePointer + headerPointer->mAllocationLength == mMemoryBuffer + mMemorySizeUsed)
    {
        mMemorySizeUsed -= (headerPointer->mAllocationLength + sizeof(linear_alloc_header));
        mMemorySizeFree += (headerPointer->mAllocationLength + sizeof(linear_alloc_header));
    }
}

void linear_memory_allocator::reset()
{
    mMemorySizeUsed = 0;
    mMemorySizeFree = mMemorySizeTotal;
}

//////////////////////////////////////////////////////////////////////////

bool heap_memory_allocator::init_allocator(unsigned int bufferSizeTotal)
{
    return true;
}

void* heap_memory_allocator::allocate(unsigned int dataLength)
{
    void* dataPoitner = malloc(dataLength);
    if (dataPoitner == nullptr && mOutOfMemoryProc)
    {
        // report overflow
        mOutOfMemoryProc(dataLength);
    }
    return dataPoitner;
}

void* heap_memory_allocator::reallocate(void* dataPointer, unsigned int dataLength)
{
    void* newPointer = realloc(dataPointer, dataLength);
    if (newPointer == nullptr && mOutOfMemoryProc)
    {
        // report overflow
        mOutOfMemoryProc(dataLength);
    }
    return newPointer;
}

void heap_memory_allocator::deallocate(void* dataPointer)
{
    if (dataPointer)
    {
        free(dataPointer);
    }
}

} // namespace cxx