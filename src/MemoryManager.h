#pragma once

#include "mem_allocators.h"

// defines system memory manager class
class MemoryManager final: public cxx::noncopyable
{
public:
    // allocates and deallocates frame heap memory

    // it's intended for objects that only should exist for a short period of time
    // all allocated memory most likely will be invalidated at start of next frame
    cxx::memory_allocator* mFrameHeapAllocator = nullptr;

    cxx::memory_allocator* mHeapAllocator = nullptr; // standard heap memory allocator

public:
    // setup memory manager internal resources
    // @returns false on error
    bool Initialize();

    void Deinit();

    // will reset previously allocated frame heap memory
    void FlushFrameHeapMemory();
};

extern MemoryManager gMemoryManager;