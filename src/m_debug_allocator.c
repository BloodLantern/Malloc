#include "m_debug_allocator.h"

void* heapDebugAlloc(size_t size)
{
    return ((char*) mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0)) + 4096 - size;
}

void heapDebugFree(void* ptr)
{
    munmap(ptr, 1);
}
