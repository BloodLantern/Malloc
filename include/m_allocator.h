#pragma once
#include <stddef.h>
#include "../src/metadata.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Memory functions
void* m_malloc(size_t size);
void* m_realloc(void* ptr, size_t size);
void* m_calloc(size_t nb, size_t size);
void m_free(void* ptr);
Metadata* get_free_block(size_t size);
int merge_blocks(Metadata* beforeBlock);
int split_block(Metadata* block, size_t neededSize);
void move_pointer(char** ptr, int change);
void* copy_value(void* newLocation, void* oldLocation);

void m_setup_hooks(void); // Hook this allocator when using standard library
void m_show_info(void); // Display allocator informations