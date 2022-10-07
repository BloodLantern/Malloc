#include <stdlib.h>
#include <stdio.h>
 
#include <m_allocator.h>
 
void test_alloc()
{
   // MALLOC
   int* a = malloc(sizeof(int));
   *a = 10;
   printf("(malloc) a = %d\n", *a);
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

void test_block_split()
{
   char* array4 = m_malloc(16 * sizeof(char));
   printf("(malloc) 4 = %p\n", array4);
   m_show_info();
}

void test_block_fusion()
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

#define ALLOC_TYPE int
#define ALLOC_NBR 1500
#define DEALLOC_START_END 0
void test_huge_allocation()
{
   ALLOC_TYPE* first[ALLOC_NBR];
   m_show_info();
   printf("Allocating %d values of size %ld...\n", ALLOC_NBR, sizeof(ALLOC_TYPE));
   for (int i = 0; i < ALLOC_NBR; i++)
      first[i] = m_malloc(sizeof(ALLOC_TYPE));
   printf("Values allocated.\n");
   m_show_info();
   // Try realloc
   printf("Freeing memory...\n");
#if DEALLOC_START_END
   for (int i = ALLOC_NBR - 1; i >= 0; i--)
#else
   for (int i = 0; i < ALLOC_NBR; i++)
#endif
      free(first[i]);
   printf("Memory freed.\n");
   m_show_info();
}
 
int main()
{
   m_setup_hooks();
   test_huge_allocation();
 
   return 0;
}