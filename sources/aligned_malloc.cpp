#include "aligned_malloc.hpp"

void * malloc_simd(const size_t size) 
{
	const size_t alignment = 16;
#if defined(__linux__)
	return aligned_alloc(alignment, size);
#elif defined(WIN32)
	return _aligned_malloc(size, alignment);
#else
    #error Unsupported platform
#endif
}

void free_simd(void * ptr) 
{
#if defined(__linux__)
    free(ptr);
#elif defined(WIN32)
	_aligned_free(ptr);
#else
    #error Unsupported platform
#endif
}

