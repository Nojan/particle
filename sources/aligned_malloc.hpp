#ifndef ALIGNED_ALLOC
#define ALIGNED_ALLOC

#include <cstdlib>

void * malloc_simd(const size_t size); 

void free_simd(void * ptr);

#endif

