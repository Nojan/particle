#include "aligned_malloc.hpp"

void * malloc_simd(const size_t size) 
{
#ifdef __linux__
    return aligned_alloc(16, size);
#endif
}

void free_simd(void * ptr) 
{
#ifdef __linux__
    free(ptr);
#endif
}

