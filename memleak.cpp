#include "memory/memleak.hpp"

void* operator new(size_t size)
{
    void* memory = MALLOC(size);

    MemleakRecorder::instance().alloc(__FILE__, __LINE__, memory);

    return memory;
}

void* operator new[](size_t size)
{
    void* memory = MALLOC(size);

    MemleakRecorder::instance().alloc(__FILE__, __LINE__, memory);

    return memory;
}

void operator delete(void* p)
{
    MemleakRecorder::instance().release(p);
    FREE(p);
}

void operator delete[](void* p)
{
    MemleakRecorder::instance().release(p);
    FREE(p);
}