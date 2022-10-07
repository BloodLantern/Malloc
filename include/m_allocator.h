#pragma once
#include <stddef.h>
#include "../src/metadata.h"

// Memory functions
void* m_malloc(size_t size);
void* m_realloc(void* ptr, size_t size);
void* m_calloc(size_t nb, size_t size);
void m_free(void* ptr);
Metadata* get_free_block(size_t size);
int merge_blocks(Metadata* beforeBlock);
int split_block(Metadata* block, size_t neededSize);

void m_setup_hooks(void); // Hook this allocator when using standard library
void m_show_info(void); // Display allocator informations