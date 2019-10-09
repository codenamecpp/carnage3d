#include "stdafx.h"
#include "MemoryManager.h"

MemoryManager gMemoryManager;

bool MemoryManager::Initialize()
{
    gConsole.LogMessage(eLogMessage_Info, "Init MemoryManager");

    if (gSystem.mConfig.mFrameHeapMemorySize > 0)
    {
        gConsole.LogMessage(eLogMessage_Info, "Frame heap memory size: %d", gSystem.mConfig.mFrameHeapMemorySize);

        mFrameHeapAllocator = new cxx::linear_memory_allocator;
        if (!mFrameHeapAllocator->init_allocator(gSystem.mConfig.mFrameHeapMemorySize))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Fail to allocate frame heap memory buffer");
            SafeDelete(mFrameHeapAllocator);
        }
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Info, "Frame heap memory disabled");
    }

    mHeapAllocator = new cxx::heap_memory_allocator;
    mHeapAllocator->init_allocator(0);
    mHeapAllocator->mOutOfMemoryProc = [](size_t allocateBytes)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot allocate %d bytes, terminating", allocateBytes);
        gSystem.Terminate();
    };

    if (mFrameHeapAllocator == nullptr)
    {
        mFrameHeapAllocator = new cxx::heap_memory_allocator;
        mFrameHeapAllocator->init_allocator(0);
    }

    // setup out of memory handler
    mFrameHeapAllocator->mOutOfMemoryProc = [](size_t allocateBytes)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot allocate %d bytes on frame heap, terminating", allocateBytes);
        gSystem.Terminate();
    };

    return true;
}

void MemoryManager::Deinit()
{
    SafeDelete(mFrameHeapAllocator);
    SafeDelete(mHeapAllocator);
}

void MemoryManager::FlushFrameHeapMemory()
{
    mFrameHeapAllocator->reset();
}