#pragma once

namespace cxx
{
    // callback proc on overflow
    using mem_allocator_out_of_memory_proc = void (*)(unsigned int allocation_size_bytes);

    // defines memory allocator interface
    class memory_allocator: public cxx::noncopyable
    {
    public:
        virtual ~memory_allocator()
        {
        }
        // setup allocator
        // @returns false on error
        virtual bool init_allocator(unsigned int bufferSizeTotal) = 0;

        // allocate at least dataLength bytes of memory
        // @param dataLength: Data length
        // @returns nullptr on out of memory
        virtual void* allocate(unsigned int dataLength) = 0;

        // reallocate previously allocated memory
        // @param dataPointer: Pointer
        // @param dataLength: New length
        // @returns nullptr on out of memory
        virtual void* reallocate(void* dataPointer, unsigned int dataLength) = 0;

        // deallocate memory
        // @param dataPointer: Pointer
        virtual void deallocate(void* dataPointer) = 0;

        // some allocators does not frees anything but rather resets free memory cursor
        virtual void reset()
        {
        }
    public:
        mem_allocator_out_of_memory_proc mOutOfMemoryProc;
    };

    // defines implementation of linear memory allocator 
    class linear_memory_allocator: public memory_allocator
    {
    public:
        ~linear_memory_allocator();

        // setup allocator
        bool init_allocator(unsigned int bufferSizeTotal) override;

        // allocate at least dataLength bytes of memory
        void* allocate(unsigned int dataLength) override;

        // reallocate previously allocated memory
        void* reallocate(void* dataPointer, unsigned int dataLength) override;

        // deallocate memory
        // does nothing
        void deallocate(void* dataPointer) override;

        // reset allocations
        void reset() override;

    private:
        unsigned int mMemorySizeTotal = 0;
        unsigned int mMemorySizeUsed = 0;
        unsigned int mMemorySizeFree = 0;
        unsigned char* mMemoryBuffer = nullptr;
    };

    // defines standard heap allocator implementation
    class heap_memory_allocator: public memory_allocator
    {
    public:
        // setup allocator
        bool init_allocator(unsigned int bufferSizeTotal) override;

        // allocate at least dataLength bytes of memory
        void* allocate(unsigned int dataLength) override;

        // reallocate previously allocated memory
        void* reallocate(void* dataPointer, unsigned int dataLength) override;

        // deallocate memory
        // does nothing
        void deallocate(void* dataPointer) override;
    };

} // namespace cxx