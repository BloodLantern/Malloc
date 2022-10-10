#include <stdlib.h>
#include <stdio.h>
 
#include <m_allocator.h>
 
void test_malloc()
{
    // MALLOC
    int* a = malloc(sizeof(int));
    *a = 10;
    printf("(malloc) a = %d\n", *a);
}

void test_block_split()
{
    char* array4 = m_malloc(16 * sizeof(char));
    printf("(malloc) 4 = %p\n", array4);
    m_show_info();
}

void test_block_merge()
{
    m_show_info();

    char* array1 = m_malloc(256 * sizeof(char));
    printf("(malloc) 1 = %p\n", array1);
    m_show_info();

    char* array2 = m_malloc(64 * sizeof(char));
    printf("(malloc) 1 = %p and 2 = %p\n", array1, array2);
    m_show_info();

    char* array3 = m_malloc(64 * sizeof(char));
    printf("(malloc) 1 = %p and 2 = %p and 3 = %p\n", array1, array2, array3);
    m_show_info();

    char* array4 = m_malloc(16 * sizeof(char));
    printf("(malloc) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();

    free(array2);
    printf("(free) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();

    free(array3);
    printf("(free) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();

    free(array1);
    printf("(free) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();

    free(array4);
    printf("(free) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();
}

void test_realloc()
{
    m_show_info();

    char* array1 = m_malloc(256 * sizeof(char));
    printf("(1. malloc) 1 = %p\n", array1);
    m_show_info();

    char* array2 = m_realloc(NULL, 64 * sizeof(char));
    printf("(2. realloc NULL) 1 = %p and 2 = %p\n", array1, array2);
    m_show_info();

    char* array3 = m_malloc(64 * sizeof(char));
    printf("(3. malloc) 1 = %p and 2 = %p and 3 = %p\n", array1, array2, array3);
    m_show_info();

    char* array4 = m_malloc(16 * sizeof(char));
    printf("(4. malloc) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();

    m_realloc(array2, 0);
    printf("(5. realloc 0) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();

    m_free(array3);
    printf("(6. free) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();

    array1 = m_realloc(array1, 512 * sizeof(char));
    printf("(7. realloc *2) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();

    m_free(array1);
    printf("(8. free) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();

    m_free(array4);
    array4[7] = 'R';
    printf("(9. free) 1 = %p and 2 = %p and 3 = %p and 4 = %p\n", array1, array2, array3, array4);
    m_show_info();
}

void test_calloc()
{
    // MALLOC
    int nbr = 5;
    int* a = calloc(nbr, sizeof(int));
    for (int i = 0; i < nbr; i++)
        printf("(calloc) a = %d\n", a[i]);
}

void test_alloc_free()
{
    m_show_info();

    char* array1 = m_malloc(256 * sizeof(char));
    printf("(malloc) 1 = %p\n", array1);
    m_show_info();

    char* array2 = m_malloc(64 * sizeof(char));
    printf("(malloc) 1 = %p and 2 = %p\n", array1, array2);
    m_show_info();

    free(array1);
    printf("(free) 1 = %p and 2 = %p\n", array1, array2);
    m_show_info();

    char* array3 = m_malloc(64 * sizeof(char));
    printf("(malloc) 1 = %p and 2 = %p and 3 = %p\n", array1, array2, array3);
    m_show_info();
}

// Var type to allocate
#define ALLOC_TYPE char
// Number of variable allocation
#define ALLOC_COUNT 100
// Size of the allocations
#define ALLOC_SIZE (sizeof(ALLOC_TYPE) * 1 * 1024 * 1024 * 1024)
// Whether to reallocate all the variables
#define REALLOC true
// Size of the reallocations
#define REALLOC_SIZE (sizeof(ALLOC_TYPE) * 2000)
// Whether to start freeing from the end or from the beginning of the variables
#define DEALLOC_START_END false

void test_huge_allocation()
{
    ALLOC_TYPE* first[ALLOC_COUNT];
    m_show_info();
    printf(ANSI_COLOR_YELLOW "Allocating %d values of size %ld...\n", ALLOC_COUNT, ALLOC_SIZE);
    for (int i = 0; i < ALLOC_COUNT; i++)
        if (i % 7 == 0)
            first[i] = m_calloc(1, ALLOC_SIZE);
        else
            first[i] = m_malloc(ALLOC_SIZE);
    printf(ANSI_COLOR_GREEN "Values allocated.\n");
    m_show_info();

#if REALLOC
    printf(ANSI_COLOR_YELLOW "Reallocating memory with new size %ld...\n", REALLOC_SIZE);
    for (int i = 0; i < ALLOC_COUNT; i++)
        first[i] = m_realloc(first[i], REALLOC_SIZE);
    printf(ANSI_COLOR_GREEN "Memory reallocated.\n");
    m_show_info();
#endif // REALLOC

	printf(ANSI_COLOR_YELLOW "Freeing memory...\n");
#if DEALLOC_START_END
   for (int i = ALLOC_COUNT - 1; i >= 0; i--)
#else
   for (int i = 0; i < ALLOC_COUNT; i++)
#endif // DEALLOC_START_END
      m_free(first[i]);
   printf(ANSI_COLOR_GREEN "Memory freed.\n");
   m_show_info();
}
 
int main()
{
    m_setup_hooks();
    test_huge_allocation();
 
    return 0;
}
