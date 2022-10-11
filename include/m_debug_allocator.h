#include <stddef.h>
#include <sys/mman.h>

void* heapDebugAlloc(size_t size);
void heapDebugFree(void* ptr);