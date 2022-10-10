# Malloc
The Malloc project consists of recoding the standard C library malloc(), calloc(), realloc() and free() functions.

# How to use ?
Simply include m_allocator.h and call m_setup_hooks() at the beginning of your program. The further malloc(), calloc(), realloc() and free() calls will result in this library's functions to be called: m_malloc(), m_calloc(), m_realloc() and m_free().