#include <stdbool.h>

typedef struct Metadata
{
   void* ptr;
   size_t size;
   bool free;
   struct Metadata* next;
} Metadata;