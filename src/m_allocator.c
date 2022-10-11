#include <malloc.h> // Nécessaire pour les hooks
#include "m_allocator.h"
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

// Return values:z
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
            Metadata* unusedData = block->ptr;
            move_pointer((char**) &unusedData, neededSize);
            // Insert it in the metadata linked list
            unusedData->next = block->next;
            block->next = unusedData;
            // Adjust the metadata values
            unusedData->free = true;
            unusedData->ptr = unusedData + 1;
            unusedData->size = block->size - neededSize - sizeof(Metadata);
            block->size = neededSize;
            // Unfree the splitted memory block
            block->free = false;
            return 1;
        }
    }
    return 0;
}

// Moves 'ptr' by 'change' bytes
void move_pointer(char** ptr, int change)
{
    *ptr += change;
}

void* copy_value(void* newLocation, void* oldLocation)
{
    int* newVal = newLocation;
    int* oldVal = oldLocation;
    *newVal = *oldVal;
    return newVal;
}

void* m_malloc(size_t size)
{
    // Try to find an available memory block
    Metadata* available = get_free_block(size);
    if (available != NULL) {
        // Try to split the memory block
        if (split_block(available, size)) {
            // If it worked, return the available memory block
            available->free = false;
            return available->ptr;
        }
    }

    // If a corresponding memory block couldn't be found, create a new one
    // Create the needed memory block for the metadata and the value
    Metadata* newData = sbrk(sizeof(Metadata) + size);
    // Place this memory block on an 8 bytes adress
    int modulo = (unsigned long) newData;
    if (modulo %= 8 > 0) {
        // Move this pointer by the needed bytes
        sbrk(modulo);
        move_pointer((char**) &newData, modulo);
        // Increase the size of the block before this one
        for (Metadata* m = metadata; m != NULL; m = m->next)
            if (m->next == NULL)
                m->size += modulo;
    }
    newData->size = size;
    newData->free = false;
    newData->next = NULL;

    // And the one for the actual pointer
    void* new = newData + 1;
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
    Metadata* data = NULL;
    for (Metadata* m = metadata; m != NULL; m = m->next)
        if (m->next != NULL)
            if (m->next->ptr == ptr) {
                data = m;
                break;
            }
    // An error happened somewhere, return NULL
    if (data == NULL)
        return NULL;

    // Try to merge this block with the ones around it
    switch (merge_blocks(data)) {
        case 1:
            // A merge was made and 'data->next' is the metadata of the new block
            if (data->next->size == size)
                return data->ptr;
            if (data->next->size > size) {
                if (split_block(data->next, size) == 0)
                    // If the split couldn't be made
                    return copy_value(m_malloc(size), ptr);
                // Else - A split was made
                // Make sure to move the line break if necessary
                if (data->next->next == NULL)
                    m_free(data->next->ptr);
                copy_value(data->ptr, ptr);
                return data->ptr;
            }
            // Block size isn't big enough
            data->next->free = true;
            return copy_value(m_malloc(size), ptr);
            
        case -1:
        case 2:
            // A merge was made and 'data' is the metadata of the new block
            if (data->size == size)
                return data->ptr;
            if (data->size > size) {
                if (split_block(data, size) == 0)
                    // If the split couldn't be made
                    return copy_value(m_malloc(size), ptr);
                // Else - A split was made
                copy_value(data->ptr, ptr);
                // Make sure to move the line break if necessary
                if (data->next->next == NULL)
                    m_free(data->next->ptr);
                return data->ptr;
            }
            // Block size isn't big enough
            data->free = true;
            return copy_value(m_malloc(size), ptr);

        case 0:
            // A merge couldn't be made, deallocate this block and allocate a new one
            data->next->free = true;
            return copy_value(m_malloc(size), ptr);
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
    // Iterate over the metadata to find the one before the one corresponding to this pointer
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
                        // No need to break
                }

                // If the block being freed is the last one
                if (next->next == NULL) {
                    // Remove the metadata from the linked list
                    if (m == next) {
                        // If the merge returned -1 or 2
                        for (Metadata* m2 = metadata; m2 != m; m2 = m2->next)
                            if (m2->next == m) {
                                m2->next = NULL;
                                break;
                            }
                    } else
                        m->next = NULL;
                    // Move the break
                    printf(ANSI_COLOR_YELLOW "Reducing program memory allocation.\n");
                    printf(ANSI_COLOR_YELLOW "Break position before reduction: %14p\n", sbrk(0));
                    brk(sbrk(0) - next->size - sizeof(Metadata));
                    printf(ANSI_COLOR_YELLOW "Break position after reduction:  %14p\n", sbrk(0));
                }
                break;
            }
}
 
void m_show_info(void)
{
    printf(ANSI_COLOR_GREEN "Metadata infos:\n");
    if (metadata == NULL) {
        printf(ANSI_COLOR_RED "\tEmpty Metadata\n");
        return;
    }

    int i = 0;
    int freeCount = 0;
    size_t freeSize = 0;
    size_t totalSize = 0;
    for (Metadata* m = metadata; m != NULL; m = m->next, i++) {
        printf(ANSI_COLOR_RESET "\tIndex " ANSI_COLOR_GREEN "%-4d"
        ANSI_COLOR_RESET ": { "
        ANSI_COLOR_YELLOW "Adress: %14p"
        ANSI_COLOR_RESET ", "
        ANSI_COLOR_BLUE "Free: %d"
        ANSI_COLOR_RESET ", "
        ANSI_COLOR_MAGENTA "Size: %8ld"
        ANSI_COLOR_RESET ", "
        ANSI_COLOR_CYAN "Pointer: %14p"
        ANSI_COLOR_RESET ", "
        ANSI_COLOR_YELLOW "Next: %14p"
        ANSI_COLOR_RESET " }\n", i, m, m->free, m->size, m->ptr, m->next);
        if (m->free) {
            freeCount++;
            freeSize += m->size;
        }
        totalSize += m->size;
    }
    printf("\tTotal     : { "
    ANSI_COLOR_BLUE "Free metadatas: %4d"
    ANSI_COLOR_RESET ", "
    ANSI_COLOR_MAGENTA "Free metadatas total size: %8ld"
    ANSI_COLOR_RESET ", "
    ANSI_COLOR_MAGENTA"Metadatas total size: %13ld"
    ANSI_COLOR_RESET " }\n", freeCount, freeSize, totalSize);
}
