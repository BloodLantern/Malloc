#include <malloc.h> // Nécessaire pour les hooks
#include <m_allocator.h>
#include <unistd.h>
#include <stdio.h>

static Metadata* metadata = NULL;
 
void* malloc_hook(size_t size, const void* caller)            { return m_malloc(size); }
void* realloc_hook(void* ptr, size_t size, const void* caller){ return m_realloc(ptr, size); }
void  free_hook(void* ptr, const void* caller)                { return m_free(ptr); }
 
void m_setup_hooks(void)
{
   __malloc_hook = malloc_hook;
   __realloc_hook = realloc_hook;
   __free_hook = free_hook;
}

Metadata* get_free_block(size_t size)
{
    // Search for an available memory block
    for (Metadata* m = metadata; m != NULL; m = m->next)
        if (m->free)
            if (m->size >= size)
                return m;
    return NULL;
}

// Return value:
// -1 if the block was merged with the one before
//  0 if the block couldn't be merged
//  1 if the block was merged with the one after
//  2 if the block was merged with the one before and the one after
int merge_blocks(Metadata* beforeBlock)
{
    Metadata* block = beforeBlock->next;
    if (beforeBlock->free) {
        // If the block before the one being freed is free
        // Adjust the size of the new block
        beforeBlock->size += block->size + sizeof(Metadata);
        beforeBlock->next = block->next;
        if (block->next != NULL)
            if (block->next->free) {
                // If the block after the one being freed is free
                // Adjust the size of the new block
                beforeBlock->size += block->next->size + sizeof(Metadata);
                beforeBlock->next = block->next->next;
                return 2;
            }
        return -1;
    } else if (block->next != NULL)
        if (block->next->free) {
            // If the block after the one being freed is free
            // Adjust the size of the new block
            block->size += block->next->size + sizeof(Metadata);
            block->next = block->next->next;
            return 1;
        }
    return 0;
}

// Return values:
// 0 if the split couldn't be made
// 1 if the split was made
int split_block(Metadata* block, size_t neededSize)
{
    // A free memory block was found
    if (block->size > neededSize) {
        // Block is bigger than the requested size. Create new metadata for the unused block part
        if (block->size - neededSize > sizeof(Metadata)) {
            // Block is big enough to keep a new metadata
            // Place the metadata pointer after the used memory block
            Metadata* unusedData = block->ptr + neededSize;
            // Insert it in the metadata linked list
            unusedData->next = block->next;
            block->next = unusedData;
            // Adjust the metadata values
            unusedData->free = true;
            unusedData->ptr = unusedData + sizeof(Metadata);
            unusedData->size = block->size - neededSize - sizeof(Metadata);
            block->size = neededSize;
            return 1;
        }
    }
    return 0;
}

void* m_malloc(size_t size)
{
    // Try to find an available memory block
    Metadata* available = get_free_block(size);
    if (available != NULL) {
        // Try to split the memory block
        split_block(available, size);
        available->free = false;
        return available->ptr;
    }

    // If a corresponding memory block couldn't be found, create a new one
    // Create the needed memory block for the metadata
    Metadata* newData = sbrk(sizeof(Metadata));
    newData->size = size;
    newData->free = false;
    newData->next = NULL;

    // And the one for the actual pointer
    void* new = sbrk(size);
    newData->ptr = new;

    // Return a NULL pointer if an error occured during the allocation
    if (new == (void*) -1)
        return NULL;

    // Create and add the metadata to the linked list
    if (metadata == NULL)
        metadata = newData;
    else
        for (Metadata* m = metadata; ; m = m->next)
            if (m->next == NULL) {
                m->next = newData;
                break;
            }

    // Eventually return the newly-created pointer
    return new;
}
 
void* m_realloc(void* ptr, size_t size)
{
    // If size == 0, deallocate the memory and return NULL
    if (size == 0) {
        m_free(ptr);
        return NULL;
    }

    // If 'ptr' is NULL, allocate the memory using malloc and return it
    if (ptr == NULL)
        return m_malloc(size);

    // Get the metadata before the one of 'ptr'
    Metadata* data;
    for (Metadata* m = metadata; m != NULL; m = m->next)
        if (m->next != NULL)
            if (m->next->ptr == ptr) {
                data = m;
                break;
            }

    // Try to merge this block with the ones around it
    switch (merge_blocks(data)) {
        case 1:
            if (data->next->size == size)
                return data->ptr;
            if (data->next->size > size) {
                split_block(data->next, size);
                return data->ptr;
            }
            // Block size isn't big enough
            data->next->free = true;
            return m_malloc(size);
            
        case -1:
        case 2:
            // If a merge was made and 'data' is the metadata of the new block
            if (data->size == size)
                return data->ptr;
            if (data->size > size) {
                split_block(data, size);
                return data->ptr;
            }
            // Block size isn't big enough
            data->next->free = true;
            return m_malloc(size);

        case 0:
            // If a merge couldn't be made, deallocate this block and allocate a new one
            data->next->free = true;
            return m_malloc(size);
    }

    return NULL;
}
 
void* m_calloc(size_t nb, size_t size)
{
    // Allocate the memory
    void* result = m_malloc(size * nb);

    // Return NULL if allocation fails
    if (result == NULL)
        return NULL;

    // Set values to 0
    size_t totalSize = size * nb;
    for (unsigned int i = 0; i < nb; i++) {
        if (totalSize >= 4) {
            ((int*) result)[i] = 0;
            totalSize -= 4;
        } else {
            ((char*) result)[i] = 0;
            totalSize--;
        }
    }

    return result;
}
 
void m_free(void* ptr)
{
    // Iterate over the metadata to find the one corresponding to this pointer
    for (Metadata* m = metadata; m != NULL; m = m->next)
        if (m->next != NULL)
            if (m->next->ptr == ptr) {
                // 'next' is the metadata of that pointer
                Metadata* next = m->next;
                next->free = true;
                
                // Try to merge free memory blocks
                switch (merge_blocks(m)) {
                    case -1:
                    case 2:
                        next = m;
                        break;
                }

                // If the block being freed is the last one
                if (next->next == NULL) {
                    // Remove the metadata from the linked list
                    if (m == next) {
                        // If the merge returned -1 or 2
                        for (Metadata* m2 = metadata; m2 != m; m2++)
                            if (m2->next == m)
                                m2->next = NULL;
                    } else
                        m->next = NULL;
                    // Move the break
                    printf("Reducing program memory allocation.\n");
                    printf("Break position before reduction: %14p\n", sbrk(0));
                    brk(sbrk(0) - next->size - sizeof(Metadata));
                    printf("Break position after reduction:  %14p\n", sbrk(0));
                }
                break;
            }
}
 
void m_show_info(void)
{
    printf("Metadata infos:\n");
    if (metadata == NULL) {
        printf("\tEmpty Metadata\n");
        return;
    }

    int i = 0;
    for (Metadata* m = metadata; m != NULL; m = m->next, i++)
        printf("\tIndex %-4d: { Adress: %14p, Free: %d, Size: %8ld, Pointer: %14p, Next: %14p }\n", i, m, m->free, m->size, m->ptr, m->next);
}
