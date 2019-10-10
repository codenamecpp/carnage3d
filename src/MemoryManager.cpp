#include "stdafx.h"
#include "MemoryManager.h"

MemoryManager gMemoryManager;

bool MemoryManager::Initialize()
{
    gConsole.LogMessage(eLogMessage_Info, "Init MemoryManager");

    if (gSystem.mConfig.mEnableFrameHeapAllocator)
    {
        gConsole.LogMessage(eLogMessage_Info, "Frame heap memory size: %d", SysMemoryFrameHeapSize);

        mFrameHeapAllocator = new cxx::linear_memory_allocator;
        if (!mFrameHeapAllocator->init_allocator(SysMemoryFrameHeapSize))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Fail to allocate frame heap memory buffer");
            SafeDelete(mFrameHeapAllocator);
        }
        else
        {
            // setup out of memory handler
            mFrameHeapAllocator->mOutOfMemoryProc = [](unsigned int allocateBytes)
            {
                gConsole.LogMessage(eLogMessage_Warning, "Cannot allocate %d bytes on frame heap", allocateBytes);
                debug_assert(false);
            };
        }
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Info, "Frame heap memory disabled");
    }

    mHeapAllocator = new cxx::heap_memory_allocator;
    mHeapAllocator->init_allocator(0);

    mHeapAllocator->mOutOfMemoryProc = [](unsigned int allocateBytes)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot allocate %d bytes", allocateBytes);
        debug_assert(false);
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
    if (mFrameHeapAllocator)
    {
        mFrameHeapAllocator->reset();
    }
}